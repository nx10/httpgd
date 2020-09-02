#ifndef HTTPGD_WEB_TASK_H
#define HTTPGD_WEB_TASK_H

#include <memory>
#include "BackgroundTask.h"
#include <boost/beast/core.hpp>
#include "HttpgdApiAsyncWatcher.h"
#include "HttpgdWebListener.h"

namespace httpgd
{
    namespace web
    {
        namespace net = boost::asio; // from <boost/asio.hpp>

        class WebServer
        {
        public:
            unsigned short port;

            WebServer(int t_threads, std::shared_ptr<HttpgdApiAsyncWatcher> t_watcher);

            void start();
            void stop();

        private:
            std::shared_ptr<HttpgdApiAsyncWatcher> m_watcher;
            int m_threads;
            std::shared_ptr<Listener> m_listener;
            net::io_context m_ioc;
        };

        class WebTask : public rsync::BackgroundTask
        {
        public:
            WebTask(int t_taskid, net::io_context *t_pioc);

        protected:
            void execute();
            void complete();

        private:
            int m_taskid;
            net::io_context *m_pioc;
        };
    } // namespace web
} // namespace httpgd

#endif