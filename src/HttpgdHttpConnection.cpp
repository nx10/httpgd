
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

#include "TargetURI.h"

#include "HttpgdHttpConnection.h"

namespace httpgd
{
    namespace http
    {
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

        namespace beast = boost::beast;   // from <boost/beast.hpp>
        namespace http = beast::http;     // from <boost/beast/http.hpp>
        namespace net = boost::asio;      // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

        const std::string HTTPGD_HTTP_VERSION("beast httpgd 1.0");

        // This function produces an HTTP response for the given
        // request. The type of the response object depends on the
        // contents of the request, so the interface requires the
        // caller to pass a generic lambda for receiving the response.
        template <
            class Body, class Allocator,
            class Send>
        void
        handle_request(
            std::shared_ptr<HttpgdServerConfig> conf,
            std::shared_ptr<HttpgdDataStore> dstore,
            http::request<Body, http::basic_fields<Allocator>> &&req,
            Send &&send)
        {
            // Returns a bad request response
            auto const bad_request =
                [&req](beast::string_view why) {
                    http::response<http::string_body> res{http::status::bad_request, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = std::string(why);
                    res.prepare_payload();
                    return res;
                };

            // Returns a not found response
            auto const not_found =
                [&req](beast::string_view target) {
                    http::response<http::string_body> res{http::status::not_found, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "The resource '" + std::string(target) + "' was not found.";
                    res.prepare_payload();
                    return res;
                };

            // Returns a server error response
            auto const server_error =
                [&req](beast::string_view what) {
                    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
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

            // todo add not_allowed / token checking
            // todo add cors

            if (uri.path == "/")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string("Hello!" + conf->host);
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/live")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());

                std::string sparams = dstore->api_state_json(conf, std::string(req[http::field::host]));
                sparams.append("/*");

                // inject params
                std::string html(conf->livehtml);
                size_t start_pos = html.find("/*SRVRPARAMS*/");
                if (start_pos != std::string::npos)
                {
                    html.replace(start_pos, sizeof("/*SRVRPARAMS*/") - 1, sparams);
                }

                res.body() = html;
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
                res.set(http::field::content_type, "image/svg+xml");
                res.keep_alive(req.keep_alive());
                res.body() = dstore->api_svg(true, cli_index, cli_width, cli_height);
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/state")
            {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req.keep_alive());
                res.body() = dstore->api_state_json();
                res.prepare_payload();
                return send(std::move(res));
            }
            else if (uri.path == "/clear")
            {
                dstore->api_clear(true);

                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, HTTPGD_HTTP_VERSION);
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req.keep_alive());
                res.body() = dstore->api_state_json();
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

                if (dstore->api_remove(true, cli_index))
                {
                    http::response<http::string_body> res{http::status::ok, req.version()};
                    res.set(http::field::server, HTTPGD_HTTP_VERSION);
                    res.set(http::field::content_type, "application/json");
                    res.keep_alive(req.keep_alive());
                    res.body() = dstore->api_state_json();
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

        //------------------------------------------------------------------------------

        // Report a failure
        void
        fail(beast::error_code ec, char const *what)
        {
            //std::cerr << what << ": " << ec.message() << "\n";
        }

        // This is the C++11 equivalent of a generic lambda.
        // The function object is used to send an HTTP message.
        template <class Stream>
        struct send_lambda
        {
            Stream &stream_;
            bool &close_;
            beast::error_code &ec_;

            explicit send_lambda(
                Stream &stream,
                bool &close,
                beast::error_code &ec)
                : stream_(stream), close_(close), ec_(ec)
            {
            }

            template <bool isRequest, class Body, class Fields>
            void
            operator()(http::message<isRequest, Body, Fields> &&msg) const
            {
                // Determine if we should close the connection after
                close_ = msg.need_eof();

                // We need the serializer here because the serializer requires
                // a non-const file_body, and the message oriented version of
                // http::write only works with const messages.
                http::serializer<isRequest, Body, Fields> sr{msg};
                http::write(stream_, sr, ec_);
            }
        };

        // Handles an HTTP server connection
        class session : public std::enable_shared_from_this<session>
        {
            // This is the C++11 equivalent of a generic lambda.
            // The function object is used to send an HTTP message.
            struct send_lambda
            {
                session &self_;

                explicit send_lambda(session &self)
                    : self_(self)
                {
                }

                template <bool isRequest, class Body, class Fields>
                void
                operator()(http::message<isRequest, Body, Fields> &&msg) const
                {
                    // The lifetime of the message has to extend
                    // for the duration of the async operation so
                    // we use a shared_ptr to manage it.
                    auto sp = std::make_shared<
                        http::message<isRequest, Body, Fields>>(std::move(msg));

                    // Store a type-erased version of the shared
                    // pointer in the class to keep it alive.
                    self_.res_ = sp;

                    // Write the response
                    http::async_write(
                        self_.stream_,
                        *sp,
                        beast::bind_front_handler(
                            &session::on_write,
                            self_.shared_from_this(),
                            sp->need_eof()));
                }
            };

            beast::tcp_stream stream_;
            beast::flat_buffer buffer_;
            std::shared_ptr<HttpgdServerConfig> conf;
            std::shared_ptr<HttpgdDataStore> dstore;
            http::request<http::string_body> req_;
            std::shared_ptr<void> res_;
            send_lambda lambda_;

        public:
            // Take ownership of the stream
            session(
                tcp::socket &&socket,
                std::shared_ptr<HttpgdServerConfig> t_conf,
                std::shared_ptr<HttpgdDataStore> t_dstore)
                : stream_(std::move(socket)), conf(t_conf), dstore(t_dstore), lambda_(*this)
            {
            }

            // Start the asynchronous operation
            void
            run()
            {
                // We need to be executing within a strand to perform async operations
                // on the I/O objects in this session. Although not strictly necessary
                // for single-threaded contexts, this example code is written to be
                // thread-safe by default.
                net::dispatch(stream_.get_executor(),
                              beast::bind_front_handler(
                                  &session::do_read,
                                  shared_from_this()));
            }

            void
            do_read()
            {
                // Make the request empty before reading,
                // otherwise the operation behavior is undefined.
                req_ = {};

                // Set the timeout.
                stream_.expires_after(std::chrono::seconds(30));

                // Read a request
                http::async_read(stream_, buffer_, req_,
                                 beast::bind_front_handler(
                                     &session::on_read,
                                     shared_from_this()));
            }

            void
            on_read(
                beast::error_code ec,
                std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);

                // This means they closed the connection
                if (ec == boost::beast::http::error::end_of_stream)
                    return do_close();

                if (ec)
                    return fail(ec, "read");

                // Send the response
                handle_request(conf, dstore, std::move(req_), lambda_);
            }

            void
            on_write(
                bool close,
                beast::error_code ec,
                std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);

                if (ec)
                    return fail(ec, "write");

                if (close)
                {
                    // This means we should close the connection, usually because
                    // the response indicated the "Connection: close" semantic.
                    return do_close();
                }

                // We're done with the response so delete it
                res_ = nullptr;

                // Read another request
                do_read();
            }

            void
            do_close()
            {
                // Send a TCP shutdown
                beast::error_code ec;
                stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

                // At this point the connection is closed gracefully
            }
        };

        listener::listener(
            net::io_context &ioc,
            tcp::endpoint endpoint,
            std::shared_ptr<HttpgdServerConfig> t_conf,
            std::shared_ptr<HttpgdDataStore> t_dstore)
            : ioc_(ioc), acceptor_(net::make_strand(ioc)), conf(t_conf), dstore(t_dstore)
        {
            beast::error_code ec;

            // Open the acceptor
            acceptor_.open(endpoint.protocol(), ec);
            if (ec)
            {
                fail(ec, "open");
                return;
            }

            // Allow address reuse
            acceptor_.set_option(net::socket_base::reuse_address(true), ec);
            if (ec)
            {
                fail(ec, "set_option");
                return;
            }

            // Bind to the server address
            acceptor_.bind(endpoint, ec);
            if (ec)
            {
                fail(ec, "bind");
                return;
            }

            // Start listening for connections
            acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
            if (ec)
            {
                fail(ec, "listen");
                return;
            }
        }

        unsigned short listener::get_port_after_bind()
        {
            return acceptor_.local_endpoint().port();
        }

        // Start accepting incoming connections
        void
        listener::run()
        {
            do_accept();
        }

        void
        listener::do_accept()
        {
            // The new connection gets its own strand
            acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                    &listener::on_accept,
                    shared_from_this()));
        }

        void
        listener::on_accept(beast::error_code ec, tcp::socket socket)
        {
            if (ec)
            {
                fail(ec, "accept");
            }
            else
            {
                // Create the session and run it
                std::make_shared<session>(
                    std::move(socket),
                    conf, dstore)
                    ->run();
            }

            // Accept another connection
            do_accept();
        }

    } // namespace http
} // namespace httpgd
