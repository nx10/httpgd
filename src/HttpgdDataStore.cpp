
#include "HttpgdDataStore.h"
#include <cmath>
#include <iostream>

// Do not include any R headers here!

namespace httpgd
{
    // safely increases numbers (wraps to 0)
    template <typename T>
    T incwrap(T t_value)
    {
        T v = t_value;
        if (v == std::numeric_limits<T>::max())
        {
            return static_cast<T>(0);
        }
        return v + 1;
    }

    HttpgdDataStore::HttpgdDataStore()
        : m_id_counter(0),
          m_pages(),
          m_upid(0),
          m_device_active(true)
    {
    }
    HttpgdDataStore::~HttpgdDataStore() = default;

    inline bool HttpgdDataStore::m_valid_index(int index)
    {
        auto psize = m_pages.size();
        return (psize > 0 && (index >= -1 && index < static_cast<int>(psize)));
    }
    inline std::size_t HttpgdDataStore::m_index_to_pos(int index)
    {
        return (index == -1 ? (m_pages.size() - 1) : index);
    }

    int HttpgdDataStore::append(double width, double height, const std::string &extra_css)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        m_pages.push_back(dc::Page(m_id_counter, width, height, extra_css));

        m_id_counter = incwrap(m_id_counter);

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
            return {10, 10};
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
        m_upid = incwrap(m_upid);
    }
    HttpgdState HttpgdDataStore::state()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        return {
            m_upid,
            m_pages.size(),
            m_device_active};
    }

    void HttpgdDataStore::set_device_active(bool t_active)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        m_device_active = t_active;
    }

    HttpgdQueryResults HttpgdDataStore::query_all()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        std::vector<long> res(m_pages.size());
        for (std::size_t i = 0; i != m_pages.size(); i++)
        {
            res[i] = m_pages[i].id;
        }
        return {{m_upid,
                 m_pages.size(),
                 m_device_active},
                res};
    }
    HttpgdQueryResults HttpgdDataStore::query_index(int t_index)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (!m_valid_index(t_index))
        {
            return {{m_upid,
                     m_pages.size(),
                     m_device_active},
                    {}};
        }
        auto index = m_index_to_pos(t_index);
        return {{m_upid,
                 m_pages.size(),
                 m_device_active},
                {m_pages[index].id}};
    }
    HttpgdQueryResults HttpgdDataStore::query_range(int t_offset, int t_limit)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (!m_valid_index(t_offset))
        {
            return {{m_upid,
                     m_pages.size(),
                     m_device_active},
                    {}};
        }
        auto index = m_index_to_pos(t_offset);
        if (t_limit < 0)
        {
            t_limit = m_pages.size();
        }
        auto end = std::min(m_pages.size(), index + static_cast<std::size_t>(t_limit));

        std::vector<long> res(end - index);
        for (std::size_t i = index; i != end; i++)
        {
            res[i - index] = m_pages[i].id;
        }
        return {{m_upid,
                 m_pages.size(),
                 m_device_active},
                res};
    }

} // namespace httpgd