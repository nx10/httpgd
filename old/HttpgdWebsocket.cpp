//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include "HttpgdWebsocketSession.h"

#include "HttpgdWebsocket.h"

//------------------------------------------------------------------------------

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
        fail(beast::error_code ec, char const *what)
        {
            //std::cerr << what << ": " << ec.message() << "\n";
        }

        // Accepts incoming connections and launches the sessions

        listener::listener(
            net::io_context &ioc,
            tcp::endpoint endpoint)
            : ioc_(ioc), acceptor_(ioc)
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
                auto a = std::make_shared<session>(std::move(socket));
                sss.push_back(a);
                a->run();
            }

            // Accept another connection
            do_accept();
        }

        
        unsigned short listener::get_port_after_bind()
        {
            return acceptor_.local_endpoint().port();
        }
        
            void listener::broadcast(std::string message) {
                for (auto s : sss) {
                    s->broadcast(message);
                }
            };

    } // namespace ws
} // namespace httpgd
