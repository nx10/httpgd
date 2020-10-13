#include <Rcpp.h>
#include "HttpgdWebServer.h"
#include <fstream>
#include <thread>
#include <sstream>

namespace httpgd
{
    namespace web
    {
        /*std::optional<std::string> readfile(std::string const &str)
        {
            std::ifstream file{str};

            if (!file.is_open())
            {
                return {};
            }

            file.seekg(0, std::ios::end);
            size_t size(static_cast<size_t>(file.tellg()));
            std::string content(size, ' ');
            file.seekg(0);
            file.read(&content[0], static_cast<std::streamsize>(size));

            return content;
        }*/

        std::string read_txt(const std::string &filepath)
        {
            std::ifstream t(filepath);
            if (t.fail()) {
                return std::string("");
            }
            std::stringstream buffer;
            buffer << t.rdbuf();
            return buffer.str();
        }

        inline bool trystod(const std::string &parse, double *out)
        {
            try
            {
                *out = std::stod(parse);
                return true;
            }
            catch (const std::exception &e)
            {
                return false;
            }
        }
        inline bool trystoi(const std::string &parse, int *out)
        {
            try
            {
                *out = std::stoi(parse);
                return true;
            }
            catch (const std::exception &e)
            {
                return false;
            }
        }

        inline void json_make_state(std::ostream &os, const std::shared_ptr<HttpgdApiAsync> &t_watcher)
        {
            os << "{ "
               << "\"upid\": " << std::to_string(t_watcher->api_upid())
               << ", \"hsize\": " << std::to_string(t_watcher->api_page_count())
               << ", \"active\": " << (t_watcher->api_active() ? "true" : "false")
               << " }";
        }

        inline std::string json_make_state(const std::shared_ptr<HttpgdApiAsync> &t_watcher)
        {
            std::ostringstream buf;
            json_make_state(buf, t_watcher);
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

                // send the file contents
                ctx.res.body() = read_txt(m_app.public_dir() + "/index.html");
                /*if (auto res = readfile(m_app.public_dir() + "/index.html"))
                {

                    ctx.res.body() = std::move(*res);
                }
                else
                {
                    throw 404;
                }*/
            });

            m_app.on_http("/state", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_state(m_watcher);
            });

            /*m_app.on_http("/test", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = std::string(m_watcher->api_server_config()->token);
            });*/

            m_app.on_http("/svg", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                // access the query parameters
                auto qparams = ctx.req.params();

                int cli_index = -1;
                double cli_width = -1;
                double cli_height = -1;

                auto find_index = qparams.find("index");
                auto find_width = qparams.find("width");
                auto find_height = qparams.find("height");

                if (find_index != qparams.end())
                {
                    std::string ptxt = find_index->second;
                    trystoi(ptxt, &cli_index);
                }
                if (find_width != qparams.end())
                {
                    std::string ptxt = find_width->second;
                    trystod(ptxt, &cli_width);
                }
                if (find_height != qparams.end())
                {
                    std::string ptxt = find_height->second;
                    trystod(ptxt, &cli_height);
                }

                std::stringstream buf;
                m_watcher->api_svg(buf, cli_index, cli_width, cli_height);

                ctx.res.set("content-type", "image/svg+xml");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = buf.str();
            });

            m_app.on_http("/remove", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                auto qparams = ctx.req.params();

                int cli_index = -1;

                auto find_index = qparams.find("index");
                if (find_index != qparams.end())
                {
                    std::string ptxt = find_index->second;
                    trystoi(ptxt, &cli_index);
                }

                if (m_watcher->api_remove(cli_index))
                {
                    ctx.res.set("content-type", "application/json");
                    ctx.res.result(OB::Belle::Status::ok);

                    ctx.res.body() = json_make_state(m_watcher);
                }
                else
                {
                    ctx.res.set("content-type", "application/json");
                    ctx.res.result(OB::Belle::Status::not_found);

                    ctx.res.body() = std::string("plot not found");
                }
            });

            m_app.on_http("/clear", OB::Belle::Method::get, [&](OB::Belle::Server::Http_Ctx &ctx) {
                if (!authorized(m_conf, ctx))
                {
                    throw 401;
                }

                m_watcher->api_clear();

                ctx.res.set("content-type", "application/json");
                ctx.res.result(OB::Belle::Status::ok);

                ctx.res.body() = json_make_state(m_watcher);
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

        void WebServer::broadcast_state()
        {
            int upid = m_watcher->api_upid();
            if (upid != m_last_upid)
            {
                m_app.channels().at("/").broadcast(json_make_state(m_watcher));
                m_last_upid = upid;
            }
        }

    } // namespace web
} // namespace httpgd
