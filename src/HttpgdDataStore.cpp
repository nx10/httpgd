
#include "HttpgdDataStore.h"
#include <cmath>
#include <iostream>

// Do not include any R headers here!

namespace httpgd
{
    inline bool HttpgdDataStore::m_valid_index(page_index_t t_index)
    {
        auto psize = m_pages.size();
        return (psize > 0 && (t_index >= -1 && t_index < static_cast<int>(psize)));
    }
    inline std::size_t HttpgdDataStore::m_index_to_pos(page_index_t t_index)
    {
        return (t_index == -1 ? (m_pages.size() - 1) : t_index);
    }

    page_index_t HttpgdDataStore::append(gvertex<double> t_size)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        m_pages.emplace_back(m_id_counter, t_size);

        m_id_counter = incwrap(m_id_counter);

        return m_pages.size() - 1;
    }
    void HttpgdDataStore::add_dc(page_index_t t_index, std::shared_ptr<dc::DrawCall> t_dc, bool t_silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].put(t_dc);
        if (!t_silent)
        {
            m_inc_upid();
        }
    }
    void HttpgdDataStore::clear(page_index_t t_index, bool t_silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].clear();
        if (!t_silent)
        {
            m_inc_upid();
        }
    }
    bool HttpgdDataStore::remove(page_index_t t_index, bool t_silent)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (!m_valid_index(t_index))
        {
            return false;
        }
        auto index = m_index_to_pos(t_index);

        m_pages.erase(m_pages.begin() + index);
        if (!t_silent) // if it was the last page
        {
            m_inc_upid();
        }
        return true;
    }
    bool HttpgdDataStore::remove_all()
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);

        if (m_pages.empty())
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
    void HttpgdDataStore::fill(page_index_t t_index, color_t t_fill)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].fill = t_fill;
    }
    void HttpgdDataStore::resize(page_index_t t_index, gvertex<double> t_size)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].size = t_size;
        m_pages[index].clear();
    }
    httpgd::gvertex<double> HttpgdDataStore::size(page_index_t t_index)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return {10, 10};
        }
        auto index = m_index_to_pos(t_index);
        return m_pages[index].size;
    }
    void HttpgdDataStore::clip(page_index_t t_index, grect<double> t_rect)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return;
        }
        auto index = m_index_to_pos(t_index);
        m_pages[index].clip(t_rect);
    }

    bool HttpgdDataStore::diff(page_index_t t_index, gvertex<double> t_size)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return false;
        }
        auto index = m_index_to_pos(t_index);

        // get current state
        gvertex<double> new_size = t_size;
        gvertex<double> old_size = m_pages[index].size;

        if (new_size.x < 0.1)
        {
            new_size.x = old_size.x;
        }
        if (new_size.y < 0.1)
        {
            new_size.y = old_size.y;
        }

        // Check if replay needed
        return (std::fabs(new_size.x - old_size.x) > 0.1 ||
                std::fabs(new_size.y - old_size.y) > 0.1);
    }
    
    bool HttpgdDataStore::render(page_index_t t_index, dc::RenderingTarget *t_renderer) 
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        if (!m_valid_index(t_index))
        {
            return false;
        }
        auto index = m_index_to_pos(t_index);
        t_renderer->render(m_pages[index]);
        return true;
    }

    boost::optional<int> HttpgdDataStore::find_index(page_id_t t_id)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        for (std::size_t i = 0; i != m_pages.size(); i++)
        {
            if (m_pages[i].id == t_id)
            {
                return static_cast<int>(i);
            }
        }
        return boost::none;
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

        std::vector<page_id_t> res(m_pages.size());
        for (std::size_t i = 0; i != m_pages.size(); i++)
        {
            res[i] = m_pages[i].id;
        }
        return {{m_upid,
                 m_pages.size(),
                 m_device_active},
                res};
    }
    HttpgdQueryResults HttpgdDataStore::query_index(page_id_t t_index)
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
    HttpgdQueryResults HttpgdDataStore::query_range(page_id_t t_offset, page_id_t t_limit)
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

        std::vector<page_id_t> res(end - index);
        for (std::size_t i = index; i != end; i++)
        {
            res[i - index] = m_pages[i].id;
        }
        return {{m_upid,
                 m_pages.size(),
                 m_device_active},
                res};
    }

    void HttpgdDataStore::extra_css(boost::optional<std::string> t_extra_css)
    {
        const std::lock_guard<std::mutex> lock(m_store_mutex);
        m_extra_css = t_extra_css;
    }

} // namespace httpgd