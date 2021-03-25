#ifndef HTTPGD_WEB_TASK_H
#define HTTPGD_WEB_TASK_H

#include <memory>
#include <belle.h>
#include "HttpgdApiAsync.h"
#include <thread>

namespace httpgd
{
    namespace web
    {
        namespace net = boost::asio; // from <boost/asio.hpp>

        class WebServer
        {
        public:

            WebServer(std::shared_ptr<HttpgdApiAsync> t_watcher);

            bool start();
            void stop();
            unsigned short port();
            void broadcast_state(const HttpgdState &state);
            void broadcast_state_current();

        private:
            std::shared_ptr<HttpgdApiAsync> m_watcher;
            std::shared_ptr<HttpgdServerConfig> m_conf;
            OB::Belle::Server m_app;
            int m_last_upid = -1;
            bool m_last_active = true;
            std::thread m_server_thread;

            void run();
        };
    } // namespace web
} // namespace httpgd

#endif