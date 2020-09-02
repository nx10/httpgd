// [[Rcpp::plugins("cpp11")]]

#include "HttpgdWebsocket.h"
#include <Rcpp.h>
#include <later_api.h>

#include "HttpgdWebsocketTask.h"

namespace httpgd
{
    namespace ws
    {
        HttpgdWebsocketTask::HttpgdWebsocketTask(const std::string &t_host, unsigned short t_port, int t_threads)
            : taskid(0), host(t_host), port(t_port), threads(t_threads)
        {
        }
        HttpgdWebsocketTask::HttpgdWebsocketTask(int t_taskid, net::io_context *t_pioc)
            : taskid(t_taskid), pioc(t_pioc)
        {
        }

        void HttpgdWebsocketTask::end()
        {
            pioc->stop();
        }

        void HttpgdWebsocketTask::execute()
        {
            if (taskid != 0) // Child process
            {
                pioc->run();
                return;
            }

            auto const address = net::ip::make_address(host);
            auto const doc_root = std::make_shared<std::string>(".");

            // The io_context is required for all I/O
            net::io_context ioc{std::max<int>(1, threads)};
            //net::io_context ioc{1};
            pioc = &ioc;

            // Create and launch a listening port
            std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

            // Create and launch a listening port
            auto lis = std::make_shared<listener>(ioc, tcp::endpoint{address, port});
            m_port = lis->get_port_after_bind();
            m_port_bound = true;
            m_lis = lis;
            lis->run();

            // Start other processes
            for (auto i = 1; i < threads; i++)
            {
                (new HttpgdWebsocketTask(i, &ioc))->begin();
            }
            ioc.run();
        }

        void HttpgdWebsocketTask::complete()
        {
            Rcpp::Rcout << "WS Task " << taskid << " stopped.\n";
            if (taskid == 0)
                Rcpp::Rcout << "WS Server stopped.\n";
        }

        int HttpgdWebsocketTask::await_port()
        {
            while (!m_port_bound)
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
            return m_port;
        }

        void HttpgdWebsocketTask::broadcast(std::string message)
        {
            m_lis->broadcast(message);
        }

    } // namespace ws
} // namespace httpgd
