
#include "HttpgdDataStore.h"
#include <cmath>
#include <iostream>

// Do not include any R headers here!

namespace httpgd
{
    HttpgdDataStore::HttpgdDataStore()
        : m_pages(), m_upid(0)
    {
    }
    HttpgdDataStore::~HttpgdDataStore() = default;

    inline bool HttpgdDataStore::m_valid_index(int index)
    {
        auto psize = m_pages.size();
        return (psize > 0 && (index >= -1 && index < static_cast<int>(psize)));
    }
    inline size_t HttpgdDataStore::m_index_to_pos(int index)
    {
        return (index == -1 ? (m_pages.size() - 1) : index);
    }

    int HttpgdDataStore::count()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        return m_pages.size();
    }
    int HttpgdDataStore::append(double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        m_pages.push_back(dc::Page(width, height));
        return m_pages.size() - 1;
    }
    void HttpgdDataStore::add_dc(int t_index, std::shared_ptr<dc::DrawCall> dc, bool silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].put(dc);
        if (!silent)
        {
            m_inc_upid();
        }
    }
    void HttpgdDataStore::clear(int t_index, bool silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].clear();
        if (!silent)
        {
            m_inc_upid();
        }
    }
    bool HttpgdDataStore::remove(int t_index, bool silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (!m_valid_index(t_index))
        {
            return false;
        }
        auto index = m_index_to_pos(t_index);

        m_pages.erase(m_pages.begin() + index);
        if (!silent) // if it was the last page
        {
            m_inc_upid();
        }
        return true;
    }
    bool HttpgdDataStore::remove_all()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (m_pages.size() == 0)
        {
            return false;
        }
        for (auto p : m_pages)
        {
            p.clear();
        }
        m_pages.clear();
        m_inc_upid();
        return true;
    }
    void HttpgdDataStore::fill(int t_index, int fill)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].fill = fill;
    }
    void HttpgdDataStore::resize(int t_index, double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].width = width;
        m_pages[index].height = height;
        m_pages[index].clear();
    }
    HttpgdDataStorePageSize HttpgdDataStore::size(int t_index)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return {0, 0};
        }
        auto index = m_index_to_pos(t_index);
        return {
            m_pages[index].width,
            m_pages[index].height};
    }
    void HttpgdDataStore::clip(int t_index, double x0, double x1, double y0, double y1)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].clip(x0, x1, y0, y1);
    }

    bool HttpgdDataStore::diff(int t_index, double width, double height)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return false;
        }
        auto index = m_index_to_pos(t_index);

        // get current state
        double old_width = m_pages[index].width;
        double old_height = m_pages[index].height;

        if (width < 0.1)
        {
            width = old_width;
        }
        if (height < 0.1)
        {
            height = old_height;
        }

        // Check if replay needed
        return (std::fabs(width - old_width) > 0.1 ||
                std::fabs(height - old_height) > 0.1);
    }
    const char *SVG_EMPTY = "<svg width=\"10\" height=\"10\" xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    void HttpgdDataStore::svg(std::ostream &os, int t_index)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            os << SVG_EMPTY;
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].build_svg(os);
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
    int HttpgdDataStore::upid()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        return m_upid;
    }

} // namespace httpgd