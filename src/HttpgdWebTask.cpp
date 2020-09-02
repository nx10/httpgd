#include <Rcpp.h>
#include "HttpgdWebTask.h"

namespace httpgd
{
    namespace web
    {

        WebTask::WebTask(int t_taskid, net::io_context *t_pioc)
            : m_taskid(t_taskid), m_pioc(t_pioc)
        {
        }

        void WebTask::execute()
        {
            m_pioc->run();
        }

        void WebTask::complete()
        {
            Rcpp::Rcout << "Web task " << m_taskid << " stopped.\n";
            //if (taskid == 0)
            //    Rcpp::Rcout << "Web server stopped.\n";
        }

        WebServer::WebServer(int t_threads, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher)
            : m_threads(std::max<int>(1, t_threads)),
              m_watcher(t_watcher),
              m_ioc{std::max<int>(1, t_threads)} // The io_context is required for all I/O
        {
        }

        void WebServer::start()
        {
            auto conf = m_watcher->api_server_config();

            auto const address = net::ip::make_address(conf->host);

            // todo: SIGTERM to kill connections?

            // Create and launch a listening port
            m_listener = std::make_shared<Listener>(m_ioc, tcp::endpoint{address, static_cast<unsigned short>(conf->port)}, m_watcher);
            port = m_listener->get_port();
            m_listener->run();

            // Start worker processes
            for (auto i = 0; i < m_threads; ++i)
            {
                (new WebTask(i, &m_ioc))->begin();
            }
        }

        void WebServer::stop()
        {
            m_ioc.stop();
        }

    } // namespace web
} // namespace httpgd
