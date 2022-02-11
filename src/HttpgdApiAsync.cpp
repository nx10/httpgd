#include "RThread.h"
#include "HttpgdApiAsync.h"
#include "AsyncUtilsDebug.h"

namespace httpgd
{

    struct AsyncApiCallIndexSizeData
    {
        HttpgdApi *api;
        int index;
        double width;
        double height;
    };
    struct AsyncApiCallIndexData
    {
        HttpgdApi *api;
        int index;
    };

    HttpgdApiAsync::HttpgdApiAsync(
        HttpgdApi *t_rdevice,
        std::shared_ptr<HttpgdServerConfig> t_svr_config,
        std::shared_ptr<HttpgdDataStore> t_data_store)
        : m_rdevice(t_rdevice),
          m_rdevice_alive(true),
          m_svr_config(t_svr_config),
          m_data_store(t_data_store)
    {
    }

    bool HttpgdApiAsync::api_remove(int index)
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive)
            return false;


        return async::r_thread([&](){
            return this->m_rdevice->api_remove(index);
        }).get();
    }
    bool HttpgdApiAsync::api_clear()
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive)
            return false;

        return async::r_thread([&](){
            return this->m_rdevice->api_clear();
        }).get();

        return true;
    }

    void HttpgdApiAsync::api_prerender(int index, double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive)
            return;

        async::r_thread([&](){
            this->m_rdevice->api_prerender(index, width, height);
        }).wait();        
    }
    
    bool HttpgdApiAsync::api_render(int index, double width, double height, dc::RenderingTarget *t_renderer, double t_scale) 
    {
        async::dbg_print("api_render");
        if (m_data_store->diff(index, {width, height}))
        {
            api_prerender(index, width, height); // use async render call
            // todo perform sync diff again and sync render svg
        }
        return m_data_store->render(index, t_renderer, t_scale);
    }

    boost::optional<int> HttpgdApiAsync::api_index(int32_t id)
    {
        return m_data_store->find_index(id);
    }

    HttpgdState HttpgdApiAsync::api_state()
    {
        return m_data_store->state();
    }
    
    HttpgdQueryResults HttpgdApiAsync::api_query_all()
    {
        return m_data_store->query_all();
    }
    HttpgdQueryResults HttpgdApiAsync::api_query_index(int index)
    {
        return m_data_store->query_index(index);
    }
    HttpgdQueryResults HttpgdApiAsync::api_query_range(int offset, int limit)
    {
        return m_data_store->query_range(offset, limit);
    }

    std::shared_ptr<HttpgdServerConfig> HttpgdApiAsync::api_server_config()
    {
        return m_svr_config;
    }

    void HttpgdApiAsync::rdevice_destructing()
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        m_rdevice_alive = false;
    }

} // namespace httpgd