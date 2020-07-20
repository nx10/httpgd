#include "RSync.h"
#include "HttpgdApiAsyncWatcher.h"

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

    HttpgdApiAsyncWatcher::HttpgdApiAsyncWatcher(
        HttpgdApi *t_rdevice,
        std::shared_ptr<HttpgdServerConfig> t_svr_config,
        std::shared_ptr<HttpgdDataStore> t_data_store)
        : m_rdevice(t_rdevice),
          m_rdevice_alive(true),
          m_svr_config(t_svr_config),
          m_data_store(t_data_store)
    {
    }

    bool HttpgdApiAsyncWatcher::api_remove(int index)
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive) return false;

        auto dat = new AsyncApiCallIndexData{
            m_rdevice,
            index};

        rsync::later([](void *t_dat) {
            auto dat = static_cast<AsyncApiCallIndexData *>(t_dat);
            HttpgdApi *api = dat->api;
            api->api_remove(dat->index);
            delete dat;
        },
                     dat, 0.0);
        rsync::awaitLater();

        return true;
    }
    bool HttpgdApiAsyncWatcher::api_clear()
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive) return false;

        rsync::later([](void *t_api) {
            auto api = static_cast<HttpgdApi *>(t_api);
            api->api_clear();
        },
                     m_rdevice, 0.0);
        rsync::awaitLater();

        return true;
    }

    void HttpgdApiAsyncWatcher::api_render(int index, double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        if (!m_rdevice_alive) return;

        auto dat = new AsyncApiCallIndexSizeData{
            m_rdevice,
            index,
            width,
            height};

        rsync::later([](void *t_dat) {
            auto dat = static_cast<AsyncApiCallIndexSizeData *>(t_dat);
            HttpgdApi *api = dat->api;
            api->api_render(dat->index, dat->width, dat->height);
            delete dat;
        },
                     dat, 0.0);
        rsync::awaitLater();
    }

    void HttpgdApiAsyncWatcher::api_svg(std::string *buf, int index, double width, double height)
    {
        if (m_data_store->diff(index, width, height))
        {
            api_render(index, width, height); // use async render call
        }
        m_data_store->svg(buf, index);
    }

    int HttpgdApiAsyncWatcher::api_upid()
    {
        return m_data_store->upid();
    }
    int HttpgdApiAsyncWatcher::api_page_count()
    {
        return m_data_store->count();
    }

    std::shared_ptr<HttpgdServerConfig> HttpgdApiAsyncWatcher::api_server_config()
    {
        return m_svr_config;
    }

    void HttpgdApiAsyncWatcher::rdevice_destructing()
    {
        const std::lock_guard<std::mutex> lock(m_rdevice_alive_mutex);
        m_rdevice_alive = false;
    }

} // namespace httpgd