//#include <Rcpp.h>
#include "HttpgdWebServer.h"
#include <fstream>
#include <thread>
#include <sstream>
#include <fmt/ostream.h>
#include <boost/optional.hpp>

namespace httpgd
{
    namespace web
    {
        boost::optional<std::string> read_txt(const std::string &filepath)
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

        inline boost::optional<std::string> param_str(OB::Belle::Request::Params params, std::string name)
        {
            auto it = params.find(name);
            if (it == params.end())
            {
                return boost::none;
            }
            return it->second;
        }
        inline boost::optional<double> param_double(OB::Belle::Request::Params params, std::string name)
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
        inline boost::optional<int> param_int(OB::Belle::Request::Params params, std::string name)
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
        inline boost::optional<long> param_long(OB::Belle::Request::Params params, std::string name)
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

        inline void json_write_state(std::ostream &buf, const HttpgdState &state)
        {
            fmt::print(buf, "{{ \"upid\": {}, \"hsize\": {}, \"active\": {} }}", state.upid, state.hsize, state.active);
        }

        inline std::string json_make_state(const HttpgdState &state)
        {
            std::stringstream buf;
            json_write_state(buf, state);
            return buf.str();
        }

        inline bool authorized(std::shared_ptr<httpgd::HttpgdServerConfig> &m_conf, OB::Belle::Server::Http_Ctx &ctx)
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
            headers.set(OB::Belle::Header::server, "httpgd 1.0");
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

            m_app.on_http("/state", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw OB::Belle::Status::unauthorized;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_state(m_watcher->api_state());
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
                    ctx.res.body() = m_watcher->api_svg(*index, p_width.get_value_or(-1), p_height.get_value_or(-1));
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
