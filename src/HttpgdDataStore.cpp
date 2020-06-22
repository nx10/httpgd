
#include "HttpgdDataStore.h"
#include "RSync.h"
#include <cmath>

// Do not include any R headers here!

namespace httpgd
{
    HttpgdDataStore::HttpgdDataStore()
        : m_pages(), m_upid(0)
    {
    }
    HttpgdDataStore::~HttpgdDataStore() = default;

    int HttpgdDataStore::page_count()
    {
        int i;
        m_page_mutex.lock();
        i = static_cast<int>(m_pages.size());
        m_page_mutex.unlock();
        return i;
    }
    int HttpgdDataStore::page_new(double width, double height)
    {
        int i;
        m_page_mutex.lock();
        m_pages.push_back(dc::Page(width, height));
        i = static_cast<int>(m_pages.size()) - 1;
        m_page_mutex.unlock();
        return i;
    }
    void HttpgdDataStore::page_put(int index, std::shared_ptr<dc::DrawCall> dc, bool silent)
    {
        m_page_mutex.lock();
        m_pages[index].put(dc);
        if (!silent)
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::page_clear(int index, bool silent)
    {
        m_page_mutex.lock();
        m_pages[index].clear();
        if (!silent)
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::page_remove(int index, bool silent)
    {
        m_page_mutex.lock();
        if (index >= static_cast<int>(m_pages.size()))
        {
            return;
        }
        //m_pages[index].clear();
        m_pages.erase(m_pages.begin() + index);
        if (!silent) // if it was the last page
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::page_remove_all()
    {
        m_page_mutex.lock();
        for (auto p : m_pages)
        {
            p.clear();
        }
        m_pages.clear();
        m_inc_upid();
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::page_fill(int index, int fill)
    {
        m_page_mutex.lock();
        m_pages[index].fill = fill;
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::m_build_svg(int index, std::string *buf)
    {
        m_page_mutex.lock();
        m_pages[index].build_svg(buf);
        m_page_mutex.unlock();
    }
    void HttpgdDataStore::page_resize(int index, double w, double h)
    {
        m_page_mutex.lock();
        m_pages[index].width = w;
        m_pages[index].height = h;
        m_pages[index].clear();
        m_page_mutex.unlock();
    }
    double HttpgdDataStore::page_get_width(int index)
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_pages[index].width;
        m_page_mutex.unlock();
        return d;
    }
    double HttpgdDataStore::page_get_height(int index)
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_pages[index].height;
        m_page_mutex.unlock();
        return d;
    }
    void HttpgdDataStore::page_clip(int index, double x0, double x1, double y0, double y1)
    {
        m_page_mutex.lock();
        m_pages[index].clip(x0, x1, y0, y1);
        m_page_mutex.unlock();
    }

    void HttpgdDataStore::m_build_state_json(std::string *buf)
    {
        buf->append("\"upid\": ").append(std::to_string(m_upid));
        m_page_mutex.lock();
        buf->append(", \"hsize\": ").append(std::to_string(m_pages.size()));
        m_page_mutex.unlock();
    }

    std::string HttpgdDataStore::api_state_json()
    {
        std::string buf;
        buf.reserve(200);
        buf.append("{ ");
        m_build_state_json(&buf);
        buf.append(" }");
        return buf;
    }
    std::string HttpgdDataStore::api_state_json(const HttpgdServerConfig *config, int port)
    {
        std::string buf;
        buf.reserve(200);
        buf.append("{ ");
        m_build_state_json(&buf);
        buf.append(", \"host\": \"").append(config->host);
        buf.append("\", \"port\": ").append(std::to_string(port));
        if (config->use_token)
        {
            buf.append(", \"token\": \"").append(config->token).append("\"");
        }
        buf.append(", \"hrecording\": ").append(config->record_history ? "true" : "false");
        buf.append(" }");
        return buf;
    }

    const char *SVG_EMPTY = "<svg width=\"10\" height=\"10\" xmlns=\"http://www.w3.org/2000/svg\"></svg>";

    // If the graphics engine scales any lower
    // it shows "figure margins too large"
    // and gets permanently in an invalid state.
    const double MIN_WIDTH = 200;
    const double MIN_HEIGHT = 200;

    std::string HttpgdDataStore::api_svg(bool async, int index, double width, double height)
    {
        rsync::lock();

        int pcount = page_count();

        if (pcount == 0)
        {
            rsync::unlock();
            return std::string(SVG_EMPTY);
        }

        if (index < 0 || index > pcount)
        {
            index = pcount - 1;
        }

        // get current state
        m_page_mutex.lock();
        double old_width = m_pages[index].width;
        double old_height = m_pages[index].height;
        m_page_mutex.unlock();

        if (width < 0.1)
        {
            width = old_width;
        }
        else if (width < MIN_WIDTH)
        {
            width = MIN_WIDTH;
        }
        if (height < 0.1)
        {
            height = old_height;
        }
        else if (height < MIN_HEIGHT)
        {
            height = MIN_HEIGHT;
        }

        // Check if replay needed
        if (std::fabs(width - old_width) > 0.1 ||
            std::fabs(height - old_height) > 0.1)
        {
            notify_replay(async, index, width, height);
        }

        std::string s = "";
        s.reserve(1000000);
        m_build_svg(index, &s);
        rsync::unlock();
        return s;
    }

    bool HttpgdDataStore::api_remove(bool async, int index)
    {
        rsync::lock();

        int pcount = page_count();
        if (index < 0)
        {
            index = pcount - 1;
        }
        if (pcount == 0 || index >= pcount)
        {
            rsync::unlock();
            return false;
        }

        page_remove(index, false);

        notify_hist_remove(async, index);
        rsync::unlock();
        return true;
    }

    bool HttpgdDataStore::api_clear(bool async)
    {
        if (page_count() > 0)
        {
            rsync::lock();
            page_remove_all();
            notify_hist_clear(async);
            rsync::unlock();
            return true;
        }
        return false;
    }

    void HttpgdDataStore::m_inc_upid()
    {
        if (m_upid < 1000000)
        {
            m_upid += 1;
        }
        else
        {
            m_upid = 0;
        }
    }
    int HttpgdDataStore::get_upid() const
    {
        return m_upid;
    }

} // namespace httpgd