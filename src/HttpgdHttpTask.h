#ifndef HTTPGD_HTTP_TASK_H
#define HTTPGD_HTTP_TASK_H

#include <memory>

//#include <later_api.h>
#include "BackgroundTask.h"
#include <boost/beast/core.hpp>
#include "HttpgdDataStore.h"
#include "HttpgdServerConfig.h"

namespace httpgd
{
    namespace http
    {
        namespace net = boost::asio;      // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

        class HttpgdHttpTask : public rsync::BackgroundTask
        {
        public:
            HttpgdHttpTask(std::shared_ptr<HttpgdServerConfig> t_conf, std::shared_ptr<HttpgdDataStore> t_data);
            void stop();
            int await_port();
            

        protected:
            void execute();
            void complete();

        private:
            std::shared_ptr<HttpgdServerConfig> m_conf;
            std::shared_ptr<HttpgdDataStore> m_data;
            net::io_context *m_pioc;
            std::atomic<bool> m_port_bound; // server is bound to a port
            unsigned int m_port;
        };
    } // namespace http
} // namespace httpgd

#endif