#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>

#include "HttpgdWebsocketSession.h"

namespace httpgd
{
    namespace ws
    {

        namespace beast = boost::beast;         // from <boost/beast.hpp>
        namespace http = beast::http;           // from <boost/beast/http.hpp>
        namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
        namespace net = boost::asio;            // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

        // Report a failure
        void
        fail2(beast::error_code ec, char const *what)
        {
            //std::cerr << what << ": " << ec.message() << "\n";
        }

        // Echoes back all received WebSocket messages
        session::session(tcp::socket &&socket)
            : ws_(std::move(socket))
        {
        }

        // Get on the correct executor
        void
        session::run()
        {
            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session. Although not strictly necessary
            // for single-threaded contexts, this example code is written to be
            // thread-safe by default.
            net::dispatch(ws_.get_executor(),
                          beast::bind_front_handler(
                              &session::on_run,
                              shared_from_this()));
        }

        // Start the asynchronous operation
        void
        session::on_run()
        {
            // Set suggested timeout settings for the websocket
            ws_.set_option(
                websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

            // Set a decorator to change the Server of the handshake
            ws_.set_option(websocket::stream_base::decorator(
                [](websocket::response_type &res) {
                    res.set(http::field::server,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                                " websocket-server-async");
                }));
            // Accept the websocket handshake
            ws_.async_accept(
                beast::bind_front_handler(
                    &session::on_accept,
                    shared_from_this()));
        }

        void
        session::on_accept(beast::error_code ec)
        {
            if (ec)
                return fail2(ec, "accept");

            // Read a message
            do_read();
        }

        void
        session::do_read()
        {
            // Read a message into our buffer
            ws_.async_read(
                buffer_,
                beast::bind_front_handler(
                    &session::on_read,
                    shared_from_this()));
        }

        void
        session::on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            // This indicates that the session was closed
            if (ec == websocket::error::closed)
                return;

            if (ec)
                fail2(ec, "read");

            // Echo the message
            ws_.text(ws_.got_text());
            ws_.async_write(
                buffer_.data(),
                beast::bind_front_handler(
                    &session::on_write,
                    shared_from_this()));
        }

        void
        session::on_write(
            beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if (ec)
                return fail2(ec, "write");

            // Clear the buffer
            buffer_.consume(buffer_.size());

            // Do another read
            do_read();
        }

        void session::broadcast(std::string message)
        {
            ws_.text(true);
            ws_.async_write(
                net::buffer(message),
                beast::bind_front_handler(
                    &session::on_write,
                    shared_from_this()));
        }

    } // namespace ws
} // namespace rbeast