#ifndef HTTPGD_HTTPGD_API_ASYNC_H
#define HTTPGD_HTTPGD_API_ASYNC_H

#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <vector>
#include "HttpgdApi.h"
#include "HttpgdCommons.h"
#include "HttpgdDataStore.h"

namespace httpgd
{
    class PlotChangedEventListener
    {
    public:
        virtual ~PlotChangedEventListener() = default; 
        virtual void plot_changed(int upid) = 0;
    };

    class HttpgdApiAsync : public HttpgdApi
    {

    public:
        std::function<void ()> broadcast_notify_change;

        HttpgdApiAsync(HttpgdApi *t_rdevice, std::shared_ptr<HttpgdServerConfig> t_svr_config, std::shared_ptr<HttpgdDataStore> t_data_store);
        virtual ~HttpgdApiAsync() = default; 

        // Calls that DO synchronize with R
        void api_prerender(int index, double width, double height) override;
        bool api_remove(int index) override;
        bool api_clear() override;


        // Calls that MAYBE synchronize with R
        bool api_render(int index, double width, double height, dc::RenderingTarget *t_renderer, double t_scale) override;
        boost::optional<int> api_index(int32_t id) override;
        
        // Calls that DONT synchronize with R
        HttpgdState api_state() override;
        HttpgdQueryResults api_query_all() override;
        HttpgdQueryResults api_query_index(int index) override;
        HttpgdQueryResults api_query_range(int offset, int limit) override;
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