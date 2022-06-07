//#include <Rcpp.h>

#define CROW_MAIN
#include <crow.h>
#include <fmt/format.h>
#include <unigd_api.h>
#include <memory>

#include "httpgd_webserver.h"
#include "httpgd_version.h"
#include "optional_lex.h"

namespace httpgd
{
    namespace web
    {
        namespace
        {
            inline boost::optional<std::string> read_txt(const std::string &filepath)
            {
                std::ifstream t(filepath);
                if (t.fail())
                {
                    return boost::none;
                }
                std::stringstream buffer;
                buffer << t.rdbuf();
                return buffer.str();
            }

            inline crow::json::wvalue device_state_json(const unigd::device_state &state)
            {
                return crow::json::wvalue({{"upid", state.upid},
                                           {"hsize", state.hsize},
                                           {"active", state.active}});
            }

            struct plot_return : public crow::returnable
            {

                std::string dump() const override
                {
                    const uint8_t *rbuf;
                    size_t rsize;
                    m_render->get_data(&rbuf, &rsize);
                    return std::string(rbuf, rbuf + rsize);
                }
                
                plot_return(unigd::renderer_info t_info, std::unique_ptr<unigd::render_data> &&t_render):
                    crow::returnable(t_info.mime),
                    m_render(std::move(t_render))
                {
                }

                ~plot_return() = default;

            private:
                const std::unique_ptr<unigd::render_data> m_render;
            };
        }

        void HttpgdLogHandler::log(std::string message, crow::LogLevel level)
        {
            std::string prefix;
            switch (level)
            {
            case crow::LogLevel::Debug:
                prefix = "DEBUG   ";
                break;
            case crow::LogLevel::Info:
                prefix = "INFO    ";
                break;
            case crow::LogLevel::Warning:
                prefix = "WARNING ";
                break;
            case crow::LogLevel::Error:
                prefix = "ERROR   ";
                break;
            case crow::LogLevel::Critical:
                prefix = "CRITICAL";
                break;
            }
            unigd::log(std::string("(") + timestamp() + std::string(") [") + prefix + std::string("] ") + message);
        }

        std::string HttpgdLogHandler::timestamp()
        {
            char date[32];
            time_t t = time(0);

            tm my_tm;

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifdef CROW_USE_LOCALTIMEZONE
            localtime_s(&my_tm, &t);
#else
            gmtime_s(&my_tm, &t);
#endif
#else
#ifdef CROW_USE_LOCALTIMEZONE
            localtime_r(&t, &my_tm);
#else
            gmtime_r(&t, &my_tm);
#endif
#endif

            size_t sz = strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &my_tm);
            return std::string(date, date + sz);
        }

        WebServer::WebServer(const HttpgdServerConfig &t_conf)
            : m_conf(t_conf),
              m_app(),
              m_mtx_update_subs(),
              m_update_subs()
        {
        }

        int WebServer::client_id()
        {
            return httpgd_client_id;
        }

        std::string WebServer::client_status()
        {
            return "httpgd " HTTPGD_VERSION;
        }

        const HttpgdServerConfig &WebServer::get_config()
        {
            return m_conf;
        }

        unsigned short WebServer::port()
        {
            m_app.wait_for_server_start();
            return m_app.port();
        }

        void WebServer::start()
        {
            m_server_thread = std::thread(&WebServer::run, this);
        }

        void WebServer::run()
        {
            crow::logger::setHandler(&m_log_handler);

            auto &cors = m_app.get_middleware<crow::CORSHandler>();
            cors
                .global()
                    .headers("Access-Control-Allow-Origin", "*");

           // CROW_ROUTE(m_app, "/")
           // ([]()
           //  { return "httpgd server is running!"; });

            CROW_ROUTE(m_app, "/live")
            ([&](const crow::request &, crow::response &res)
             {
                const auto filepath = std::string(m_conf.wwwpath) + "/index.html";
                res.set_static_file_info_unsafe(filepath);
                res.end(); });

            CROW_ROUTE(m_app, "/state")
            ([&]()
             {
                if (auto api_locked = api.lock()) {
                    const auto state = api_locked->api_state();
                    return crow::response(device_state_json(state));
                } 
                return crow::response(404); });

            CROW_ROUTE(m_app, "/renderers")
            ([&]()
             {
                std::vector<unigd::renderer_info> renderers;
                if (unigd::get_renderer_list(&renderers))
                {

                    std::vector<crow::json::wvalue> a;
                    a.reserve(renderers.size());
                    for (auto it = renderers.begin(); it != renderers.end(); ++it)
                    {
                        a.push_back(crow::json::wvalue({ 
                            {"id", it->id},
                            {"mime", it->mime},
                            {"ext", it->fileext},
                            {"name", it->name},
                            {"type", it->type},
                            {"bin", !it->text},
                            {"descr", it->description}
                        }));
                    }
                    return crow::response(crow::json::wvalue({{"renderers", a}}));
                }
                return crow::response(404); });

            CROW_ROUTE(m_app, "/plots")
            ([&](const crow::request &req)
             {
                const auto p_index = param_to<int>(req.url_params.get("index"));
                const auto p_limit = param_to<int>(req.url_params.get("limit"));

                 if (auto api_locked = api.lock()) {
                    unigd::device_api_query_result qr;
                    if (p_limit)
                    {
                        qr = api_locked->api_query_range(p_index.get_value_or(0), *p_limit);
                    }
                    else if (p_index)
                    {
                        qr = api_locked->api_query_index(*p_index);
                    }
                    else
                    {
                        qr = api_locked->api_query_all();
                    }

                    std::vector<crow::json::wvalue> plot_list;
                    plot_list.reserve(qr.ids.size());
                    for (auto it = qr.ids.begin(); it != qr.ids.end(); ++it)
                    {
                        plot_list.push_back(crow::json::wvalue({ 
                            {"id", fmt::format("{}", *it)}
                        }));
                    }
                    return crow::response(crow::json::wvalue({
                        {"state", device_state_json(qr.state)},
                        {"plots", plot_list}
                        }));
                }
                return crow::response(404); });

            CROW_ROUTE(m_app, "/plot")
            ([&](const crow::request &req)
             {
                const auto p_width = param_to<int>(req.url_params.get("width"));
                const auto p_height = param_to<int>(req.url_params.get("height"));
                double width, height, zoom;
                if (p_width && p_height)
                {
                    zoom = param_to<double>(req.url_params.get("zoom")).get_value_or(1);
                    width = (*p_width) / zoom;
                    height = (*p_height) / zoom;
                } 
                else
                {
                    zoom = 1;
                    width = p_width.get_value_or(-1);
                    height = p_height.get_value_or(-1);
                }
                const auto p_id = param_to<long>(req.url_params.get("id")).get_value_or(-1); // todo?
                const auto p_renderer = param_to<std::string>(req.url_params.get("renderer")).get_value_or("svg");
                const auto p_download = param_to<const char*>(req.url_params.get("download"));
                if (auto api_locked = api.lock()) {
                    unigd::renderer_info rinfo;
                    if (!unigd::get_renderer_info(p_renderer, &rinfo))
                    {
                        return crow::response(404);
                    }

                    auto render = api_locked->api_render(p_renderer, p_id, width, height, zoom);

                    if (!render)
                    {
                        return crow::response(404);
                    }

                    const uint8_t *rbuf;
                    size_t rsize;
                    render->get_data(&rbuf, &rsize);

                    auto res = crow::response(plot_return(rinfo, std::move(render)));
                    if (p_download) {
                        res.add_header("Content-Disposition", fmt::format("attachment; filename=\"{}\"", *p_download));
                    }
                    return res;
                }

                return crow::response(404); });

            CROW_ROUTE(m_app, "/info")
            ([&]()
             { return crow::json::wvalue({
                 {"id", m_conf.id},
                 {"version", "httpgd " HTTPGD_VERSION}
                 }); });

            CROW_ROUTE(m_app, "/").websocket()
                .onopen([&](crow::websocket::connection& conn) {
                    CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
                    std::lock_guard<std::mutex> _(m_mtx_update_subs);
                    m_update_subs.insert(&conn);
                })
                .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
                    CROW_LOG_INFO << "websocket connection closed: " << reason;
                    std::lock_guard<std::mutex> _(m_mtx_update_subs);
                    m_update_subs.erase(&conn);
                })
                .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
                    std::lock_guard<std::mutex> _(m_mtx_update_subs);
                    for (auto u : m_update_subs)
                        if (is_binary)
                            u->send_binary(data);
                        else
                            u->send_text(data);
                });

            CROW_ROUTE(m_app, "/<str>")
            ([&](crow::response &res, std::string s)
             {
                CROW_LOG_INFO << "static: " << s;
                const auto filepath = std::string(m_conf.wwwpath) + "/" + s;
                res.set_static_file_info_unsafe(filepath);
                res.end(); });

            m_app.bindaddr(m_conf.host).port(m_conf.port).multithreaded().run();
        }

        void WebServer::close()
        {
            m_app.stop();

            if (m_server_thread.joinable())
            {
                m_server_thread.join();
            }
        }

        void WebServer::broadcast_state(const unigd::device_state &t_state)
        {
            std::lock_guard<std::mutex> _(m_mtx_update_subs);
            for (auto u : m_update_subs)
            {
                u->send_text(device_state_json(t_state).dump());
            }
        }

        void WebServer::broadcast_state_current()
        {
            if (auto api_locked = api.lock()) {
                unigd::device_state state = api_locked->api_state();
                broadcast_state(state);
            }
        }

    } // namespace web
} // namespace httpgd
