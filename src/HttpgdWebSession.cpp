
#include "HttpgdWebSession.h"
#include <memory>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/websocket.hpp>

#include "TargetURI.h"
#include <fstream>

#include "HttpgdApiAsyncWatcher.h"

namespace httpgd
{
    namespace web
    {
        // Websocket session

        // Take ownership of the socket
        WsSession::WsSession(tcp::socket &&socket, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
            : ws_(std::move(socket)), m_watcher(t_watcher)
        {
            m_watcher->add_listener(shared_from_this());
        }

        // Start the asynchronous accept operation
        template <class Body, class Allocator>
        void WsSession::do_accept(http::request<Body, http::basic_fields<Allocator>> req)
        {
            // Set suggested timeout settings for the websocket
            ws_.set_option(
                websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

            // Set a decorator to change the Server of the handshake
            ws_.set_option(websocket::stream_base::decorator(
                [](websocket::response_type &res) {
                    res.set(http::field::server,
                            //std::string(BOOST_BEAST_VERSION_STRING) +
                            std::string("Hello todo") +
                                " advanced-server");
                }));

            // Accept the websocket handshake
            ws_.async_accept(
                req,
                beast::bind_front_handler(
                    &WsSession::on_accept,
                    shared_from_this()));
        }

        void WsSession::on_accept(beast::error_code ec)
        {
            if (ec)
                return; //fail(ec, "accept");

            // Read a message
            do_read();
        }

        void WsSession::do_read()
        {
            // Read a message into our buffer
            ws_.async_read(
                buffer_,
                beast::bind_front_handler(
                    &WsSession::on_read,
                    shared_from_this()));
        }

        void WsSession::on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            // This indicates that the websocket_session was closed
            if (ec == websocket::error::closed)
                return;

            //if (ec)
            //    fail(ec, "read");

            // Echo the message
            ws_.text(ws_.got_text());
            ws_.async_write(
                buffer_.data(),
                beast::bind_front_handler(
                    &WsSession::on_write,
                    shared_from_this()));
        }

        void WsSession::plot_changed(int upid)
        {

            ws_.async_write(
                boost::asio::buffer( "Yoyoyo\n" ),
                beast::bind_front_handler(
                    &WsSession::on_write,
                    shared_from_this()));
        }

        void WsSession::on_write(
            beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            //if (ec)
            //    return fail(ec, "write");

            // Clear the buffer
            buffer_.consume(buffer_.size());

            // Do another read
            do_read();
        }

        // HTTP session

        std::string read_txt(const std::string &filepath)
        {
            std::ifstream t(filepath);
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

        inline void json_state(std::string *buf, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
        {
            buf->append("\"upid\": ").append(std::to_string(t_watcher->api_upid()));
            buf->append(", \"hsize\": ").append(std::to_string(t_watcher->api_page_count()));
        }
        inline void json_state(std::string *buf, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher, std::string host)
        {
            json_state(buf, t_watcher);
            buf->append(", \"host\": \"").append(host).append("\"");
            auto config = t_watcher->api_server_config();
            if (config->use_token)
            {
                buf->append(", \"token\": \"").append(config->token).append("\"");
            }
        }
        inline std::string json_make_state(std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
        {
            std::string buf;
            buf.reserve(200);
            buf.append("{ ");
            json_state(&buf, t_watcher);
            buf.append(" }");
            return buf;
        }
        inline std::string json_make_state_full(std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher, std::string host)
        {
            std::string buf;
            buf.reserve(200);
            buf.append("{ ");
            json_state(&buf, t_watcher, host);
            buf.append(" }");
            return buf;
        }
        
        const std::string HTTPGD_HTTP_VERSION("beast httpgd 1.0a");

        // This function produces an HTTP response for the given
        // request. The type of the response object depends on the
        // contents of the request, so the interface requires the
        // caller to pass a generic lambda for receiving the response.
        template <
            class Body, class Allocator,
            class Send>
        void
        handle_request(
            std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher,
            http::request<Body, http::basic_fields<Allocator>> &&req,
            Send &&send)
        {
            auto conf = t_watcher->api_server_config();

            // Returns a bad request response
            auto const bad_request =
                [&req, &conf](beast::string_view why) {
                    http::response<http::string_body> res{http::status::bad_request, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    if (conf->cors)
                    {
                        res.set(http::field::access_control_allow_origin, "*");
                    }
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = std::string(why);
                    res.prepare_payload();
                    return res;
                };

            // Returns a not found response
            auto const not_found =
                [&req, &conf](beast::string_view target) {
                    http::response<http::string_body> res{http::status::not_found, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    if (conf->cors)
                    {
                        res.set(http::field::access_control_allow_origin, "*");
                    }
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "The resource '" + std::string(target) + "' was not found.";
                    res.prepare_payload();
                    return res;
                };

            // Returns a server error response
            auto const server_error =
                [&req, &conf](beast::string_view what) {
                    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    if (conf->cors)
                    {
                        res.set(http::field::access_control_allow_origin, "*");
                    }
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "An error occurred: '" + std::string(what) + "'";
                    res.prepare_payload();
                    return res;
                };

            // Make sure we can handle the method
            if (req.method() != http::verb::get)
                return send(bad_request("Unknown HTTP-method"));

            TargetURI uri(std::string(req.target()));

            // check token
            if (conf->use_token &&
                !((req["X-HTTPGD-TOKEN"] == conf->token) ||
                  uri.qparams["token"] == conf->token))
            {
                http::response<http::string_body> res{http::status::unauthorized, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string("Unauthorized");
                res.prepare_payload();
                return send(std::move(res));
            }

            // todo add not_allowed / token checking
            // todo add cors

            if (uri.path == "/")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string("httpgd server running.");
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/live")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());

                std::string path(conf->wwwpath + "/index.html");
                std::string html(read_txt(path));

                // inject params
                std::string host = std::string(req[http::field::host]);
                std::string sparams = json_make_state_full(t_watcher, host); 
                sparams.append("/*");

                size_t start_pos = html.find("/*SRVRPARAMS*/");
                if (start_pos != std::string::npos)
                {
                    html.replace(start_pos, sizeof("/*SRVRPARAMS*/") - 1, sparams);
                }
                if (conf->use_token)
                {
                    start_pos = html.find("<!--LIVEJSSCIPTTAG-->");
                    if (start_pos != std::string::npos)
                    {
                        html.replace(start_pos, sizeof("<!--LIVEJSSCIPTTAG-->") - 1, "<script src=\"httpgd.js?token=" + conf->token + "\"></script><!--");
                    }
                }

                res.body() = html;
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/httpgd.js")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());

                // inject params
                std::string path(conf->wwwpath + "/httpgd.js");
                std::string txt(read_txt(path));

                res.body() = txt;
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/svg")
            {
                int cli_index = -1;
                double cli_width = -1;
                double cli_height = -1;

                auto find_index = uri.qparams.find("index");
                auto find_width = uri.qparams.find("width");
                auto find_height = uri.qparams.find("height");

                if (find_index != uri.qparams.end())
                {
                    std::string ptxt = find_index->second;
                    trystoi(ptxt, &cli_index);
                }
                if (find_width != uri.qparams.end())
                {
                    std::string ptxt = find_width->second;
                    trystod(ptxt, &cli_width);
                }
                if (find_height != uri.qparams.end())
                {
                    std::string ptxt = find_height->second;
                    trystod(ptxt, &cli_height);
                }

                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "image/svg+xml");
                res.keep_alive(req.keep_alive());

                std::string buf = "";
                buf.reserve(1000000);
                t_watcher->api_svg(&buf, cli_index, cli_width, cli_height);
                res.body() = buf;
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/state")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req.keep_alive());
                res.body() = json_make_state(t_watcher);
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/clear")
            {
                t_watcher->api_clear();

                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                if (conf->cors)
                {
                    res.set(http::field::access_control_allow_origin, "*");
                }
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req.keep_alive());
                res.body() = json_make_state(t_watcher);
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/remove")
            {
                int cli_index = -1;

                auto find_index = uri.qparams.find("index");
                if (find_index != uri.qparams.end())
                {
                    std::string ptxt = find_index->second;
                    trystoi(ptxt, &cli_index);
                }

                if (t_watcher->api_remove(cli_index))
                {
                    http::response<http::string_body> res{http::status::ok, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    if (conf->cors)
                    {
                        res.set(http::field::access_control_allow_origin, "*");
                    }
                    res.set(http::field::content_type, "application/json");
                    res.keep_alive(req.keep_alive());
                    res.body() = json_make_state(t_watcher);
                    res.prepare_payload();
                    return send(std::move(res));
                }
                else
                {
                    return send(not_found("plot"));
                }
            }

            return send(not_found("ressource"));
        }

        namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

        HttpSession::queue::queue(HttpSession &self)
            : self_(self)
        {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        bool HttpSession::queue::is_full() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool HttpSession::queue::on_write()
        {
            BOOST_ASSERT(!items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if (!items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template <bool isRequest, class Body, class Fields>
        void HttpSession::queue::operator()(http::message<isRequest, Body, Fields> &&msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                HttpSession &self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(
                    HttpSession &self,
                    http::message<isRequest, Body, Fields> &&msg)
                    : self_(self), msg_(std::move(msg))
                {
                }

                void
                operator()()
                {
                    http::async_write(
                        self_.stream_,
                        msg_,
                        beast::bind_front_handler(
                            &HttpSession::on_write,
                            self_.shared_from_this(),
                            msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(
                boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1)
                (*items_.front())();
        }

        // Take ownership of the socket
        HttpSession::HttpSession(
            tcp::socket &&socket,
            std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher) 
            : stream_(std::move(socket)), m_watcher(t_watcher), queue_(*this)
        {
        }

        // Start the session
        void HttpSession::run()
        {
            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session. Although not strictly necessary
            // for single-threaded contexts, this example code is written to be
            // thread-safe by default.
            net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(
                    &HttpSession::do_read,
                    this->shared_from_this()));
        }

        void HttpSession::do_read()
        {
            // Construct a new parser for each message
            parser_.emplace();

            // Apply a reasonable limit to the allowed size
            // of the body in bytes to prevent abuse.
            parser_->body_limit(10000);

            // Set the timeout.
            stream_.expires_after(std::chrono::seconds(30));

            // Read a request using the parser-oriented interface
            http::async_read(
                stream_,
                buffer_,
                *parser_,
                beast::bind_front_handler(
                    &HttpSession::on_read,
                    shared_from_this()));
        }

        void HttpSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            // This means they closed the connection
            if (ec == http::error::end_of_stream)
                return do_close();

            if (ec)
                return; //fail(ec, "read");

            // See if it is a WebSocket Upgrade
            if (websocket::is_upgrade(parser_->get()))
            {
                // Create a websocket session, transferring ownership
                // of both the socket and the HTTP request.
                std::make_shared<WsSession>(
                    stream_.release_socket(), m_watcher)
                    ->do_accept(parser_->release());
                return;
            }

            // Send the response
            handle_request(m_watcher, parser_->release(), queue_);

            // If we aren't at the queue limit, try to pipeline another request
            if (!queue_.is_full())
                do_read();
        }

        void HttpSession::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if (ec)
                return; // fail(ec, "write");

            if (close)
            {
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                return do_close();
            }

            // Inform the queue that a write completed
            if (queue_.on_write())
            {
                // Read another request
                do_read();
            }
        }

        void HttpSession::do_close()
        {
            // Send a TCP shutdown
            beast::error_code ec;
            stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

            // At this point the connection is closed gracefully
        }

    } // namespace web
} // namespace httpgd
