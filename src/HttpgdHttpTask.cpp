
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
            std::shared_ptr<HttpgdServerConfig> t_conf,
            std::shared_ptr<HttpgdDataStore> t_data)
            : m_conf(t_conf), m_data(t_data),
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
                const auto port = static_cast<unsigned short>(m_conf->port);
                const auto address = net::ip::make_address(m_conf->host);

                net::io_context ioc{1};
                m_pioc = &ioc;

                // Create and launch a listening port
                auto lis = std::make_shared<listener>(
                    ioc,
                    tcp::endpoint{address, (unsigned short)port},
                    m_conf, m_data);
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
