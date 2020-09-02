#ifndef RBEAST_WS_SERVER_ASYNC_SESSION_H
#define RBEAST_WS_SERVER_ASYNC_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "Broadcast.h"

namespace httpgd
{
    namespace ws
    {

        namespace beast = boost::beast;         // from <boost/beast.hpp>
        namespace net = boost::asio;            // from <boost/asio.hpp>
        namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
        using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

        // Echoes back all received WebSocket messages
        class session : public std::enable_shared_from_this<session>, public Broadcastable
        {
            websocket::stream<beast::tcp_stream> ws_;
            beast::flat_buffer buffer_;

        public:
            // Take ownership of the socket
            explicit session(tcp::socket &&socket);

            // Get on the correct executor
            void
            run();

            // Start the asynchronous operation
            void
            on_run();

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

            void broadcast(std::string message) override;
        };
    } // namespace ws

} // namespace rbeast

#endif