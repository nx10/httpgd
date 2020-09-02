
#include <thread>
#include <iostream>

#include "HttpgdHttpConnection.h"

#include "HttpgdHttpTask.h"

namespace httpgd
{
    namespace http
    {
        namespace net = boost::asio; // from <boost/asio.hpp>

        HttpgdHttpTask::HttpgdHttpTask(
            std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
            : m_watcher(t_watcher),
              m_pioc(nullptr),
              m_port_bound(false), m_port(-1)
        {
        }

        void HttpgdHttpTask::stop()
        {
            m_pioc->stop();
        }

        void HttpgdHttpTask::execute()
        {
            try
            {
                const auto conf = m_watcher->api_server_config();
                const auto port = static_cast<unsigned short>(conf->port);
                const auto address = net::ip::make_address(conf->host);

                net::io_context ioc{1};
                m_pioc = &ioc;

                // Create and launch a listening port
                auto lis = std::make_shared<listener>(
                    ioc,
                    tcp::endpoint{address, (unsigned short)port},
                    m_watcher);
                m_port = lis->get_port_after_bind();
                m_port_bound = true;
                lis->run();

                
                // Start other processes
                ioc.run();
            }
            catch (const std::exception &e)
            {
                //
            }
        }

        void HttpgdHttpTask::complete()
        {
            // Rcpp::Rcout << "Server stopped.\n";
        }

        int HttpgdHttpTask::await_port()
        {
            while (!m_port_bound)
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
            return m_port;
        }

    } // namespace http
} // namespace httpgd
