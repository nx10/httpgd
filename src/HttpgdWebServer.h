#ifndef HTTPGD_WEB_TASK_H
#define HTTPGD_WEB_TASK_H

#include <memory>
#include <belle.h>
#include <thread>
#include <unigd_api/client.h>

namespace httpgd
{
    namespace web
    {
        constexpr int httpgd_client_id = 104;

        struct HttpgdServerConfig
        {
            std::string host;
            int port;
            std::string wwwpath;
            bool cors;
            bool use_token;
            std::string token;
            bool record_history;
            bool webserver;
            bool silent;
            std::string id;
        };

        namespace net = boost::asio; // from <boost/asio.hpp>

        class WebServer : public unigd::graphics_client
        {
        public:
            WebServer(const HttpgdServerConfig &t_config);

            void start() override;
            void close() override;
            void broadcast_state_current() override;
            int client_id() override;
            const HttpgdServerConfig &get_config();

            unsigned short port();
            void broadcast_state(const unigd::device_state &state);

        private:
            HttpgdServerConfig m_conf;
            OB::Belle::Server m_app;
            int m_last_upid = -1;
            bool m_last_active = true;
            std::thread m_server_thread;

            void run();
        };
    } // namespace web
} // namespace httpgd

#endif