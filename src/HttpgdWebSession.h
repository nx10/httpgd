
#ifndef HTTPGD_WEB_SESSION_H
#define HTTPGD_WEB_SESSION_H

#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "HttpgdApiAsyncWatcher.h"

namespace httpgd
{
    namespace web
    {
        namespace beast = boost::beast;         // from <boost/beast.hpp>
        namespace http = beast::http;           // from <boost/beast/http.hpp>
        namespace net = boost::asio;            // from <boost/asio.hpp>
        namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
        using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

        // Echoes back all received WebSocket messages
        class WsSession : public std::enable_shared_from_this<WsSession>, public PlotChangedEventListener
        {
            websocket::stream<beast::tcp_stream> ws_;
            beast::flat_buffer buffer_;
            std::shared_ptr<HttpgdApiAsyncWatcher> m_watcher;

        public:
            // Take ownership of the socket
            explicit WsSession(tcp::socket &&socket, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher);

            // Start the asynchronous accept operation
            template <class Body, class Allocator>
            void do_accept(http::request<Body, http::basic_fields<Allocator>> req);

            void plot_changed(int upid) override;

        private:
            void
            on_accept(beast::error_code ec);

            void
            do_read();

            void
            on_read(
                beast::error_code ec,
                std::size_t bytes_transferred);

            void
            on_write(
                beast::error_code ec,
                std::size_t bytes_transferred);
        };

        // Handles an HTTP server connection
        class HttpSession : public std::enable_shared_from_this<HttpSession>
        {
            // This queue is used for HTTP pipelining.
            class queue
            {
                enum
                {
                    // Maximum number of responses we will queue
                    limit = 8
                };

                // The type-erased, saved work item
                struct work
                {
                    virtual ~work() = default;
                    virtual void operator()() = 0;
                };

                HttpSession &self_;
                std::vector<std::unique_ptr<work>> items_;

            public:
                explicit queue(HttpSession &self);

                // Returns `true` if we have reached the queue limit
                bool is_full() const;

                // Called when a message finishes sending
                // Returns `true` if the caller should initiate a read
                bool on_write();

                // Called by the HTTP handler to send a response.
                template <bool isRequest, class Body, class Fields>
                void operator()(http::message<isRequest, Body, Fields> &&msg);
            };

            beast::tcp_stream stream_;
            beast::flat_buffer buffer_;
            std::shared_ptr<HttpgdApiAsyncWatcher> m_watcher;
            queue queue_;

            // The parser is stored in an optional container so we can
            // construct it from scratch it at the beginning of each new message.
            boost::optional<http::request_parser<http::string_body>> parser_;

        public:
            // Take ownership of the socket
            HttpSession(
                tcp::socket &&socket,
                std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher);

            // Start the session
            void run();

        private:
            void do_read();

            void on_read(beast::error_code ec, std::size_t bytes_transferred);

            void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);
            void do_close();
        };

    } // namespace web
} // namespace httpgd

#endif