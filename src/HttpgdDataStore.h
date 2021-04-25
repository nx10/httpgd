#ifndef HTTPGD_SERVER_STORE_H
#define HTTPGD_SERVER_STORE_H

#include "DrawData.h"
#include "HttpgdApi.h"
#include "HttpgdCommons.h"
#include "HttpgdGeom.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace httpgd
{
    using page_id_t = int32_t;
    using page_index_t = int;

    class HttpgdDataStore
    {
    public:
        boost::optional<page_index_t> find_index(page_id_t t_id);

        bool diff(page_index_t t_index, gvertex<double> t_size);
        std::string svg(page_index_t t_index);
        bool render(page_index_t t_index, dc::RenderingTarget *t_renderer);

        page_index_t append(gvertex<double> t_size);
        void clear(page_index_t t_index, bool t_silent);
        bool remove(page_index_t t_index, bool t_silent);
        bool remove_all();
        void resize(page_index_t t_index, gvertex<double> t_size);
        gvertex<double> size(page_index_t t_index);

        void fill(page_index_t t_index, color_t t_fill);
        void add_dc(page_index_t t_index, std::shared_ptr<dc::DrawCall> t_dc, bool t_silent);
        void clip(page_index_t t_index, grect<double> t_rect);

        HttpgdState state();
        void set_device_active(bool t_active);

        HttpgdQueryResults query_all();
        HttpgdQueryResults query_index(page_index_t t_index);
        HttpgdQueryResults query_range(page_index_t t_offset, page_index_t t_limit);

        void extra_css(boost::optional<std::string> t_extra_css);

    private:
        std::mutex m_store_mutex;

        page_id_t m_id_counter = 0;
        std::vector<dc::Page> m_pages;
        int m_upid = 0;
        bool m_device_active = true;

        boost::optional<std::string> m_extra_css;

        void m_inc_upid();

        inline bool m_valid_index(page_index_t t_index);
        inline size_t m_index_to_pos(page_index_t t_index);
        
    };

} // namespace httpgd

#endif // HTTPGD_SERVER_H