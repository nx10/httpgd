#ifndef HTTPGD_SERVER_TASK_H
#define HTTPGD_SERVER_TASK_H

#define HTTPGD_SERVER_TASK_IMPL 2

#if HTTPGD_SERVER_TASK_IMPL == 2
#include "BackgroundTask.h"
#else
#include <later_api.h>
#endif

#include "lib/cpphttplib.h"
#include "HttpgdDataStore.h"

namespace httpgd
{
    class HttpgdServerTask
#if HTTPGD_SERVER_TASK_IMPL == 1
        : public later::BackgroundTask
#else
        : public rsync::BackgroundTask
#endif
    {
    public:
        HttpgdServerTask(
            HttpgdDataStore *t_data,
            const HttpgdServerConfig *t_config);

        void stop();

        int await_port();

    protected:
        void execute();
        void complete();

    private:
        std::atomic<bool> m_is_bound_to_port; // server is bound to a port
        std::atomic<int> m_bound_port;
        const HttpgdServerConfig *m_config;
        HttpgdDataStore *m_data;
        httplib::Server m_svr;
        bool m_prepare_req(const httplib::Request &req, httplib::Response &res) const;
        void m_run_server();
    };

    bool check_server_started(const std::string &host, int port);

} // namespace httpgd
#endif