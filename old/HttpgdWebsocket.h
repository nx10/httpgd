#ifndef RBEAST_WS_SERVER_ASYNC_H
#define RBEAST_WS_SERVER_ASYNC_H

#include <boost/beast/core.hpp>
#include <vector>
#include <memory>

#include "Broadcast.h"

namespace httpgd
{
    namespace ws
    {

        namespace beast = boost::beast;   // from <boost/beast.hpp>
        namespace net = boost::asio;      // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

        // Accepts incoming connections and launches the sessions
        class listener : public std::enable_shared_from_this<listener>, public Broadcastable
        {
            net::io_context &ioc_;
            tcp::acceptor acceptor_;

        public:
            listener(
                net::io_context &ioc,
                tcp::endpoint endpoint);

            // Start accepting incoming connections
            void
            run();

            unsigned short get_port_after_bind();

            std::vector<std::shared_ptr<Broadcastable>> sss;
            
            void broadcast(std::string message) override;

        private:
            void
            do_accept();

            void
            on_accept(beast::error_code ec, tcp::socket socket);
        };
    } // namespace ws

} // namespace rbeast

#endif