//#include <Rcpp.h>
#include "HttpgdWebServer.h"
#include <fstream>
#include <thread>
#include <sstream>
#include <fmt/ostream.h>
#include <boost/optional.hpp>

#include "HttpgdVersion.h"
//#include "RendererSvg.h"
#include <unigd_api.h>
#include <fmt/format.h>

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

        static inline void json_write_state(std::ostream &buf, const unigd::device_state &state)
        {
            fmt::print(buf, "{{ \"upid\": {}, \"hsize\": {}, \"active\": {} }}", state.upid, state.hsize, state.active);
        }

        static inline std::string json_make_state(const unigd::device_state &state)
        {
            std::stringstream buf;
            json_write_state(buf, state);
            return buf.str();
        }

        static inline void json_write_info(std::ostream &buf, const HttpgdServerConfig &t_conf)
        {
            fmt::print(buf, R""({{ "id": "{}", "version": {{ "httpgd": "{}", "boost": "{}", "cairo": "{}" }} }})"", 
                t_conf.id, 
                HTTPGD_VERSION, 
                HTTPGD_VERSION_BOOST, 
                HTTPGD_VERSION_CAIRO
            );
        }

        static inline std::string json_make_info(const HttpgdServerConfig &t_conf)
        {
            std::stringstream buf;
            json_write_info(buf, t_conf);
            return buf.str();
        }

        template<typename T>
        static inline bool authorized(const HttpgdServerConfig &m_conf, T &ctx)
        {
            if (!m_conf.use_token)
            {
                return true;
            }
            auto token_header = ctx.req.find("x-httpgd-token");
            if ((token_header != ctx.req.end() && token_header->value() == m_conf.token))
            {
                return true;
            }

            auto &qparams = ctx.req.params();
            auto token_param = qparams.find("token");
            if ((token_param != qparams.end() && token_param->second == m_conf.token))
            {
                return true;
            }

            return false;
        }

        WebServer::WebServer(const HttpgdServerConfig &t_conf)
            : m_conf(t_conf),
              m_app()
        {
        }

        int WebServer::client_id()
        {
            return httpgd_client_id;
        }
        
        std::string WebServer::client_status()
        {
            return "httpgd 2.0";
        }

        const HttpgdServerConfig &WebServer::get_config()
        {
            return m_conf;
        }

        unsigned short WebServer::port()
        {
            return m_app.port();
        }

        void WebServer::start()
        {

            m_app.address(m_conf.host);
            m_app.port(m_conf.port);

            if (!m_app.available())
            {
                // port blocked
                return;//TODO false;
            }

            // set default http headers
            OB::Belle::Headers headers;
            headers.set(OB::Belle::Header::server, "httpgd " HTTPGD_VERSION);
            //headers.set(OB::Belle::Header::cache_control, "private; max-age=0");
            if (m_conf.cors)
            {
                headers.set(OB::Belle::Header::access_control_allow_origin, "*");
                headers.set(OB::Belle::Header::access_control_allow_methods, "GET, POST, PATCH, PUT, DELETE, OPTIONS");
                headers.set(OB::Belle::Header::access_control_allow_headers, "Origin, Content-Type, X-Auth-Token, X-HTTPGD-TOKEN");
            }
            m_app.http_headers(headers);

            m_app.public_dir(m_conf.wwwpath);

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

                if (auto api_locked = api.lock()) {
                    ctx.res.body() = json_make_state(api_locked->api_state());
                } else {
                    ctx.res.body() = ""; //TODO
                }
            });

            m_app.on_http("/renderers", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                fmt::memory_buffer buf;
                fmt::format_to(std::back_inserter(buf), "{{\n \"renderers\": [\n");
                
                std::vector<unigd::renderer_info> renderers;
                if (unigd::get_renderer_list(&renderers))
                {
                    for (auto it = renderers.begin(); it != renderers.end(); ++it)
                    {
                        fmt::format_to(std::back_inserter(buf), R""(  {{ "id": "{}", "mime": "{}", "ext": "{}", "name": "{}", "type": "{}", "bin": {}, "descr": "{}" }})"",
                            it->id,
                            it->mime,
                            it->fileext,
                            it->name,
                            it->type,
                            !it->text,
                            it->description
                        );
                        if (std::next(it) != renderers.end())
                        {
                            fmt::format_to(std::back_inserter(buf), ",\n");
                        }
                    }
                }

                fmt::format_to(std::back_inserter(buf), "\n ]\n}}");

                ctx.res.body() = fmt::to_string(buf);
            });

            m_app.on_http("/plots", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_index = param_int(qparams, "index");
                auto p_limit = param_int(qparams, "limit");

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
                } else {
                    ctx.res.result(OB::Belle::Status::internal_server_error);
                    ctx.res.body() = "";
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
                auto p_id = param_long(qparams, "id").get_value_or(-1); // todo?
                auto p_renderer = param_str(qparams, "renderer").get_value_or("svg");
                auto p_download = param_str(qparams, "download");

                if (auto api_locked = api.lock()) {
                    boost::optional<int> index;
                    /*if (p_id)
                    {
                        index = api_locked->api_index(*p_id);
                    }
                    else
                    {
                        index = param_int(qparams, "index").get_value_or(-1);
                    }*/

                    unigd::renderer_info rinfo;
                    if (!unigd::get_renderer_info(p_renderer, &rinfo))
                    {
                        throw OB::Belle::Status::not_found;
                    }

                    

                    const auto render = api_locked->api_render(p_renderer, p_id, width, height, zoom);

                    if (render)
                    {
                        const uint8_t *rbuf;
                        size_t rsize;
                        render->get_data(&rbuf, &rsize);

                        ctx.res.result(OB::Belle::Status::ok);
                        ctx.res.set("content-type", rinfo.mime);
                        if (p_download) {
                            ctx.res.set("Content-Disposition", fmt::format("attachment; filename=\"{}\"", *p_download));
                        }

                        ctx.res.body().assign(rbuf, rbuf+rsize);// = renderer->get_string();
                        
                    }
                    else
                    {
                        throw OB::Belle::Status::not_found;
                    }
                } else {
                    throw OB::Belle::Status::internal_server_error;
                }
            });

            m_app.on_http("/remove", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                auto qparams = ctx.req.params();
                auto p_id = param_long(qparams, "id");

                if (!p_id)
                {
                    throw OB::Belle::Status::not_found; 
                }

                if (auto api_locked = api.lock()) {
                    boost::optional<int> index;
                    /*if (p_id)
                    {
                        index = m_watcher->api_index(*p_id);
                    }
                    else
                    {
                        index = param_int(qparams, "index").get_value_or(-1);
                    }*/

                    if (api_locked->api_remove(*p_id))
                    {
                        ctx.res.set("content-type", "application/json");
                        ctx.res.result(OB::Belle::Status::ok);

                        ctx.res.body() = json_make_state(api_locked->api_state());
                    }
                    else
                    {
                        throw OB::Belle::Status::not_found;
                    }
                } else {
                    throw OB::Belle::Status::internal_server_error;
                }
            });

            m_app.on_http("/clear", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                if (auto api_locked = api.lock()) {
                    api_locked->api_clear(); 

                    ctx.res.set("content-type", "application/json");
                    ctx.res.result(OB::Belle::Status::ok);

                    ctx.res.body() = json_make_state(api_locked->api_state());
                } else {
                    throw OB::Belle::Status::internal_server_error;
                }
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

            //return true; TODO
        }

        void WebServer::run()
        {
            m_app.listen();
        }

        void WebServer::close()
        {
            // todo: send SIGINT/SIGTERM for clean shutdown?
            m_app.io().stop();
            if (m_server_thread.joinable())
            {
                m_server_thread.join();
            }
        }

        void WebServer::broadcast_state(const unigd::device_state &state)
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
            if (auto api_locked = api.lock()) {
                unigd::device_state state = api_locked->api_state();
                broadcast_state(state);
            } else {
                //TODO
            }
        }

    } // namespace web
} // namespace httpgd
