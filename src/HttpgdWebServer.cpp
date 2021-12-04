//#include <Rcpp.h>
#include "HttpgdWebServer.h"
#include <fstream>
#include <thread>
#include <sstream>
#include <fmt/ostream.h>
#include <boost/optional.hpp>

#include "HttpgdVersion.h"
#include "RendererSvg.h"

namespace httpgd
{
    namespace web
    {
        static boost::optional<std::string> read_txt(const std::string &filepath)
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

        static inline boost::optional<std::string> param_str(OB::Belle::Request::Params params, std::string name)
        {
            auto it = params.find(name);
            if (it == params.end())
            {
                return boost::none;
            }
            return it->second;
        }
        static inline boost::optional<double> param_double(OB::Belle::Request::Params params, std::string name)
        {
            auto it = params.find(name);
            if (it == params.end())
            {
                return boost::none;
            }
            try
            {
                double val = std::stod(it->second);
                return val;
            }
            catch (const std::exception &e)
            {
                return boost::none;
            }
        }
        static inline boost::optional<int> param_int(OB::Belle::Request::Params params, std::string name)
        {
            auto it = params.find(name);
            if (it == params.end())
            {
                return boost::none;
            }
            try
            {
                int val = std::stoi(it->second);
                return val;
            }
            catch (const std::exception &e)
            {
                return boost::none;
            }
        }
        static inline boost::optional<long> param_long(OB::Belle::Request::Params params, std::string name)
        {
            auto it = params.find(name);
            if (it == params.end())
            {
                return boost::none;
            }
            try
            {
                long val = std::stol(it->second);
                return val;
            }
            catch (const std::exception &e)
            {
                return boost::none;
            }
        }

        static inline void json_write_state(std::ostream &buf, const HttpgdState &state)
        {
            fmt::print(buf, "{{ \"upid\": {}, \"hsize\": {}, \"active\": {} }}", state.upid, state.hsize, state.active);
        }

        static inline std::string json_make_state(const HttpgdState &state)
        {
            std::stringstream buf;
            json_write_state(buf, state);
            return buf.str();
        }

        static inline void json_write_info(std::ostream &buf, std::shared_ptr<httpgd::HttpgdServerConfig> t_conf)
        {
            fmt::print(buf, R""({{ "id": "{}", "version": {{ "httpgd": "{}", "boost": "{}", "cairo": "{}" }} }})"", 
                t_conf->id, 
                HTTPGD_VERSION, 
                HTTPGD_VERSION_BOOST, 
                HTTPGD_VERSION_CAIRO
            );
        }

        static inline std::string json_make_info(std::shared_ptr<httpgd::HttpgdServerConfig> t_conf)
        {
            std::stringstream buf;
            json_write_info(buf, t_conf);
            return buf.str();
        }

        template<typename T>
        static inline bool authorized(std::shared_ptr<httpgd::HttpgdServerConfig> &m_conf, T &ctx)
        {
            if (!m_conf->use_token)
            {
                return true;
            }
            auto token_header = ctx.req.find("x-httpgd-token");
            if ((token_header != ctx.req.end() && token_header->value() == m_conf->token))
            {
                return true;
            }

            auto &qparams = ctx.req.params();
            auto token_param = qparams.find("token");
            if ((token_param != qparams.end() && token_param->second == m_conf->token))
            {
                return true;
            }

            return false;
        }

        WebServer::WebServer(std::shared_ptr<HttpgdApiAsync> t_watcher)
            : m_watcher(t_watcher),
              m_conf(t_watcher->api_server_config()),
              m_app()
        {
        }

        unsigned short WebServer::port()
        {
            return m_app.port();
        }

        bool WebServer::start()
        {

            m_app.address(m_conf->host);
            m_app.port(m_conf->port);

            if (!m_app.available())
            {
                // port blocked
                return false;
            }

            // set default http headers
            OB::Belle::Headers headers;
            headers.set(OB::Belle::Header::server, "httpgd " HTTPGD_VERSION);
            //headers.set(OB::Belle::Header::cache_control, "private; max-age=0");
            if (m_conf->cors)
            {
                headers.set(OB::Belle::Header::access_control_allow_origin, "*");
                headers.set(OB::Belle::Header::access_control_allow_methods, "GET, POST, PATCH, PUT, DELETE, OPTIONS");
                headers.set(OB::Belle::Header::access_control_allow_headers, "Origin, Content-Type, X-Auth-Token, X-HTTPGD-TOKEN");
            }
            m_app.http_headers(headers);

            m_app.public_dir(m_conf->wwwpath);

            m_app.websocket(true);
            m_app.signals({SIGINT, SIGTERM});
            // set the on signal callback
            m_app.on_signal([&](auto ec, auto sig) {
                // print out the signal received
                //std::cerr << "\nSignal " << sig << "\n";

                // get the io_context and safely stop the server
                m_app.io().stop();
            });
            m_app.channels()["/"] = OB::Belle::Server::Channel();

            m_app.on_http("/", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                ctx.res.set("content-type", "text/html");
                ctx.res.result(OB::Belle::Status::ok);
                ctx.res.body() = std::string("httpgd server running.");
            });

            // cors preflight
            m_app.on_http("^/.*$", OB::Belle::Method::options, [&](OB::Belle::Server::Http_Ctx &ctx) {
                ctx.res.result(OB::Belle::Status::ok);
            });

            m_app.on_http("/live", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                ctx.res.set("content-type", "text/html");
                ctx.res.result(OB::Belle::Status::ok);

                std::string filepath = m_app.public_dir() + "/index.html";
                ctx.res.body() = read_txt(filepath).get_value_or(
                    fmt::format("<html><body><b>ERROR:</b> File not found ({}).<br>Please reload package.</body></html>", filepath));
            });

            m_app.on_http("/info", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_info(m_conf);
            });

            m_app.on_http("/state", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_state(m_watcher->api_state());
            });

            m_app.on_http("/renderers", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                fmt::memory_buffer buf;
                fmt::format_to(buf, "{{\n \"renderers\": [\n");
                
                const auto &renderers = RendererManager::defaults();

                for (auto it = renderers.string_renderers().begin(); it != renderers.string_renderers().end(); it++) 
                {
                    fmt::format_to(buf, R""(  {{ "id": "{}", "mime": "{}", "ext": "{}", "name": "{}", "type": "{}", "bin": false, "descr": "{}" }})"",
                        it->second.id,
                        it->second.mime,
                        it->second.fileext,
                        it->second.name,
                        it->second.type,
                        it->second.description
                    );
                    if (std::next(it) != renderers.string_renderers().end())
                    {
                        fmt::format_to(buf, ",\n");
                    }
                }
                for (auto it = renderers.binary_renderers().begin(); it != renderers.binary_renderers().end(); it++) 
                {
                    fmt::format_to(buf, ",\n");
                    fmt::format_to(buf, R""(  {{ "id": "{}", "mime": "{}", "ext": "{}", "name": "{}", "type": "{}", "bin": true, "descr": "{}" }})"",
                        it->second.id,
                        it->second.mime,
                        it->second.fileext,
                        it->second.name,
                        it->second.type,
                        it->second.description
                    );
                }
                
                fmt::format_to(buf, "\n ]\n}}");

                ctx.res.body() = fmt::to_string(buf);
            });

            m_app.on_http("/plots", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                HttpgdQueryResults qr;

                auto qparams = ctx.req.params();
                auto p_index = param_int(qparams, "index");
                auto p_limit = param_int(qparams, "limit");

                if (p_limit)
                {
                    qr = m_watcher->api_query_range(p_index.get_value_or(0), *p_limit);
                }
                else if (p_index)
                {
                    qr = m_watcher->api_query_index(*p_index);
                }
                else
                {
                    qr = m_watcher->api_query_all();
                }

                std::stringstream buf;
                buf << "{ \"state\": ";
                json_write_state(buf, qr.state);
                buf << ", \"plots\": [";

                for (const auto &id : qr.ids)
                {
                    if (&id != &qr.ids[0])
                    {
                        buf << ", ";
                    }
                    fmt::print(buf, "{{ \"id\": \"{}\" }}", id);
                }
                buf << "] }";

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);
                ctx.res.body() = buf.str();
            });

            m_app.on_http("/svg", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_width = param_double(qparams, "width");
                auto p_height = param_double(qparams, "height");
                double width, height, zoom;
                if (p_width && p_height)
                {
                    zoom = param_double(qparams, "zoom").get_value_or(1);
                    width = (*p_width) / zoom;
                    height = (*p_height) / zoom;
                } 
                else
                {
                    zoom = 1;
                    width = p_width.get_value_or(-1);
                    height = p_height.get_value_or(-1);
                }
                auto p_id = param_long(qparams, "id");

                boost::optional<int> index;
                if (p_id)
                {
                    index = m_watcher->api_index(*p_id);
                }
                else
                {
                    index = param_int(qparams, "index").get_value_or(-1);
                }

                if (index)
                {
                    ctx.res.set("content-type", "image/svg+xml");
                    ctx.res.result(OB::Belle::Status::ok);
                    dc::RendererSVG renderer(boost::none);
                    if (m_watcher->api_render(*index, width, height, &renderer, zoom)) {
                        ctx.res.body() = renderer.get_string();
                    } else {
                        throw OB::Belle::Status::not_found;
                    }
                }
                else
                {
                    throw OB::Belle::Status::not_found;
                }
            });

            m_app.on_http("/plot", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_width = param_double(qparams, "width");
                auto p_height = param_double(qparams, "height");
                double width, height, zoom;
                if (p_width && p_height)
                {
                    zoom = param_double(qparams, "zoom").get_value_or(1);
                    width = (*p_width) / zoom;
                    height = (*p_height) / zoom;
                } 
                else
                {
                    zoom = 1;
                    width = p_width.get_value_or(-1);
                    height = p_height.get_value_or(-1);
                }
                auto p_id = param_long(qparams, "id");
                auto p_renderer = param_str(qparams, "renderer").get_value_or("svg");
                auto p_download = param_str(qparams, "download");

                boost::optional<int> index;
                if (p_id)
                {
                    index = m_watcher->api_index(*p_id);
                }
                else
                {
                    index = param_int(qparams, "index").get_value_or(-1);
                }

                if (index)
                {
                    ctx.res.set("content-type", "image/png");
                    ctx.res.result(OB::Belle::Status::ok);

                    const auto find_renderer = RendererManager::defaults().find_string(p_renderer);
                    if (!find_renderer) {
                        throw OB::Belle::Status::not_found;
                    }
                    const auto renderer = (*find_renderer).renderer();
                    if (m_watcher->api_render(*index, width, height, renderer.get(), zoom)) {
                        ctx.res.set("content-type", (*find_renderer).mime);
                        if (p_download) {
                            ctx.res.set("Content-Disposition", fmt::format("attachment; filename=\"{}\"", *p_download));
                        }
                        ctx.res.body() = renderer->get_string();
                    } else {
                        throw OB::Belle::Status::not_found;
                    }
                }
                else
                {
                    throw OB::Belle::Status::not_found;
                }
            });
            m_app.on_http("/plot", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx_dyn &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_width = param_double(qparams, "width");
                auto p_height = param_double(qparams, "height");
                double width, height, zoom;
                if (p_width && p_height)
                {
                    zoom = param_double(qparams, "zoom").get_value_or(1);
                    width = (*p_width) / zoom;
                    height = (*p_height) / zoom;
                } 
                else
                {
                    zoom = 1;
                    width = p_width.get_value_or(-1);
                    height = p_height.get_value_or(-1);
                }
                auto p_id = param_long(qparams, "id");
                auto p_renderer = param_str(qparams, "renderer").get_value_or("png");
                auto p_download = param_str(qparams, "download");

                boost::optional<int> index;
                if (p_id)
                {
                    index = m_watcher->api_index(*p_id);
                }
                else
                {
                    index = param_int(qparams, "index").get_value_or(-1);
                }

                if (index)
                {
                    ctx.res.result(OB::Belle::Status::ok);

                    const auto find_renderer = RendererManager::defaults().find_binary(p_renderer);
                    if (!find_renderer) {
                        throw OB::Belle::Status::not_found;
                    }
                    const auto renderer = (*find_renderer).renderer();
                    if (m_watcher->api_render(*index, width, height, renderer.get(), zoom)) {
                        ctx.res.set("content-type", (*find_renderer).mime);
                        if ((*find_renderer).id.rfind("svgz", 0) == 0) {
                            ctx.res.set("Content-Encoding", "gzip"); // todo
                        }
                        if (p_download) {
                            ctx.res.set("Content-Disposition", fmt::format("attachment; filename=\"{}\"", *p_download));
                        }
                        ctx.res.body() = renderer->get_binary();
                    } else {
                        throw OB::Belle::Status::not_found;
                    }
                }
                else
                {
                    throw OB::Belle::Status::not_found;
                }
            });

            m_app.on_http("/remove", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_id = param_long(qparams, "id");

                boost::optional<int> index;
                if (p_id)
                {
                    index = m_watcher->api_index(*p_id);
                }
                else
                {
                    index = param_int(qparams, "index").get_value_or(-1);
                }

                if (index && m_watcher->api_remove(*index))
                {
                    ctx.res.set("content-type", "application/json");
                    ctx.res.result(OB::Belle::Status::ok);

                    ctx.res.body() = json_make_state(m_watcher->api_state());
                }
                else
                {
                    throw OB::Belle::Status::not_found;
                }
            });

            m_app.on_http("/clear", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                m_watcher->api_clear();

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_state(m_watcher->api_state());
            });

            // set custom error callback
            m_app.on_http_error([](OB::Belle::Server::Http_Ctx &ctx) {
                // stringstream to hold the response
                std::stringstream res;
                res
                    << "Status: " << ctx.res.result_int() << "\n"
                    << "Reason: " << ctx.res.result() << "\n";

                // set http response headers
                ctx.res.set("content-type", "text/plain");

                // echo the http status code
                ctx.res.body() = res.str();
            });

            // handle ws connections to index room '/'
            m_app.on_websocket("/",
                               // on data: called after every websocket read
                               [](OB::Belle::Server::Websocket_Ctx &ctx) {
                                   // register the route
                                   // data will be broadcasted in the websocket connect and disconnect handlers
                                   //ctx.send("test1");
                                   //ctx.broadcast("test2");
                               });

            m_server_thread = std::thread(&WebServer::run, this);

            return true;
        }

        void WebServer::run()
        {
            m_app.listen();
        }

        void WebServer::stop()
        {
            // todo: send SIGINT/SIGTERM for clean shutdown?
            m_app.io().stop();
            if (m_server_thread.joinable())
            {
                m_server_thread.join();
            }
        }

        void WebServer::broadcast_state(const HttpgdState &state)
        {
            if (state.upid != m_last_upid || state.active != m_last_active)
            {
                m_app.channels().at("/").broadcast(json_make_state(state));
                m_last_upid = state.upid;
                m_last_active = state.active;
            }
        }

        void WebServer::broadcast_state_current()
        {
            HttpgdState state = m_watcher->api_state();
            broadcast_state(state);
        }

    } // namespace web
} // namespace httpgd
