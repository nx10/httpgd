#include "AsyncLater.h"
#include "HttpgdApiAsync.h"

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

        auto dat = new AsyncApiCallIndexData{
            m_rdevice,
            index};

        asynclater::later([](void *t_dat) {
            auto dat = static_cast<AsyncApiCallIndexData *>(t_dat);
            HttpgdApi *api = dat->api;
            api->api_remove(dat->index);
            delete dat;
        },
                     dat, 0.0);
        asynclater::awaitLater();

        return true;
    }
    bool HttpgdApiAsync::api_clear()
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive)
            return false;

        asynclater::later([](void *t_api) {
            auto api = static_cast<HttpgdApi *>(t_api);
            api->api_clear();
        },
                     m_rdevice, 0.0);
        asynclater::awaitLater();

        return true;
    }

    void HttpgdApiAsync::api_render(int index, double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive)
            return;

        auto dat = new AsyncApiCallIndexSizeData{
            m_rdevice,
            index,
            width,
            height};

        asynclater::later([](void *t_dat) {
            auto dat = static_cast<AsyncApiCallIndexSizeData *>(t_dat);
            HttpgdApi *api = dat->api;
            api->api_render(dat->index, dat->width, dat->height);
            delete dat;
        },
                     dat, 0.0);
        asynclater::awaitLater();
    }

    void HttpgdApiAsync::api_svg(std::ostream &os, int index, double width, double height)
    {
        if (m_data_store->diff(index, width, height))
        {
            api_render(index, width, height); // use async render call
            // todo perform sync diff again and sync render svg
        }
        m_data_store->svg(os, index);
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