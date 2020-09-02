#include "HttpgdWebListener.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

#include "HttpgdWebSession.h"

namespace httpgd
{
    namespace web
    {
        Listener::Listener(
            net::io_context &t_ioc,
            tcp::endpoint t_endpoint, 
            std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
            : m_ioc(t_ioc), m_acceptor(net::make_strand(t_ioc)), m_watcher(t_watcher)
        {
            beast::error_code ec;

            // Open the acceptor
            m_acceptor.open(t_endpoint.protocol(), ec);
            if (ec)
            {
                // fail(ec, "open");
                return;
            }

            // Allow address reuse
            m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
            if (ec)
            {
                // fail(ec, "set_option");
                return;
            }

            // Bind to the server address
            m_acceptor.bind(t_endpoint, ec);
            if (ec)
            {
                // fail(ec, "bind");
                return;
            }

            // Start listening for connections
            m_acceptor.listen(
                net::socket_base::max_listen_connections, ec);
            if (ec)
            {
                // fail(ec, "listen");
                return;
            }
        }

        // Start accepting incoming connections
        void Listener::run()
        {
            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session. Although not strictly necessary
            // for single-threaded contexts, this code is written to be
            // thread-safe by default.
            net::dispatch(
                m_acceptor.get_executor(),
                beast::bind_front_handler(
                    &Listener::do_accept,
                    this->shared_from_this()));
        }

        unsigned short Listener::get_port()
        {
            return m_acceptor.local_endpoint().port();
        }

        void Listener::do_accept()
        {
            // The new connection gets its own strand
            m_acceptor.async_accept(
                net::make_strand(m_ioc),
                beast::bind_front_handler(
                    &Listener::on_accept,
                    shared_from_this()));
        }

        void Listener::on_accept(beast::error_code ec, tcp::socket socket)
        {
            if (ec)
            {
                // fail(ec, "accept");
            }
            else
            {
                // Create the http session and run it
                std::make_shared<HttpSession>(
                    std::move(socket),
                    m_watcher)
                    ->run();
            }

            // Accept another connection
            do_accept();
        }

    } // namespace web
} // namespace httpgd