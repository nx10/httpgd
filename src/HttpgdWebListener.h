
#ifndef HTTPGD_WEB_LISTENER_H
#define HTTPGD_WEB_LISTENER_H

#include <memory>
#include <boost/beast/core.hpp>

#include "HttpgdApiAsyncWatcher.h"

namespace httpgd
{
    namespace web
    {
        namespace beast = boost::beast;   // from <boost/beast.hpp>
        namespace net = boost::asio;      // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

        // Accepts incoming connections and launches the sessions
        class Listener : public std::enable_shared_from_this<Listener>
        {
            net::io_context &m_ioc;
            tcp::acceptor m_acceptor;
            std::shared_ptr<HttpgdApiAsyncWatcher> m_watcher;

        public:
            Listener(
                net::io_context &t_ioc,
                tcp::endpoint t_endpoint,
                std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher);

            // Start accepting incoming connections
            void run();
            unsigned short get_port();

        private:
            void do_accept();

            void on_accept(beast::error_code t_ec, tcp::socket t_socket);
        };
    } // namespace web
} // namespace httpgd

#endif