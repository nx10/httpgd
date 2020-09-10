#ifndef HTTPGD_WEB_TASK_H
#define HTTPGD_WEB_TASK_H

#include <memory>
#include <belle.h>
#include "HttpgdApiAsyncWatcher.h"
#include <thread>

namespace httpgd
{
    namespace web
    {
        namespace net = boost::asio; // from <boost/asio.hpp>

        class WebServer
        {
        public:

            WebServer(std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher);

            bool start();
            void stop();
            unsigned short port();
            void broadcast_upid(int upid);

        private:
            std::shared_ptr<HttpgdApiAsyncWatcher> m_watcher;
            std::shared_ptr<HttpgdServerConfig> m_conf;
            OB::Belle::Server m_app;
            int m_last_upid = -1;
            std::thread m_server_thread;

            void run();
        };
    } // namespace web
} // namespace httpgd

#endif