#ifndef HTTPGD_HTTPGD_API_ASYNC_WATCHER_H
#define HTTPGD_HTTPGD_API_ASYNC_WATCHER_H

#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <vector>
#include "HttpgdApi.h"
#include "HttpgdServerConfig.h"
#include "HttpgdDataStore.h"

namespace httpgd
{
    class PlotChangedEventListener
    {
    public:
        virtual void plot_changed(int upid) = 0;
    };

    class HttpgdApiAsyncWatcher : public HttpgdApi
    {

    public:
        std::function<void ()> broadcast_notify_change;

        HttpgdApiAsyncWatcher(HttpgdApi *t_rdevice, std::shared_ptr<HttpgdServerConfig> t_svr_config, std::shared_ptr<HttpgdDataStore> t_data_store);
        virtual ~HttpgdApiAsyncWatcher() = default; 

        // Calls that DO synchronize with R
        void api_render(int index, double width, double height) override;
        bool api_remove(int index) override;
        bool api_clear() override;

        // Calls that MAYBE synchronize with R
        void api_svg(std::ostream &os, int index, double width, double height) override;
        
        // Calls that DONT synchronize with R
        int api_upid() override;
        int api_page_count() override;
        std::shared_ptr<HttpgdServerConfig> api_server_config() override;

        // this will block when a operation is running in another thread that needs the r device to be alive
        void rdevice_destructing();

    private:
        HttpgdApi *m_rdevice;
        bool m_rdevice_alive;
        std::mutex m_rdevice_alive_mutex;
        
        std::shared_ptr<HttpgdServerConfig> m_svr_config;
        std::shared_ptr<HttpgdDataStore> m_data_store;
    };
} // namespace httpgd

#endif