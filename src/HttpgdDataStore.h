#ifndef HTTPGD_SERVER_STORE_H
#define HTTPGD_SERVER_STORE_H

#include <functional>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>

#include "HttpgdServerConfig.h"
#include "DrawData.h"

namespace httpgd
{

    class HttpgdDataStore
    {
    public:
        // callbacks
        std::function<void(bool async, int index, double width, double height)> notify_replay;
        std::function<void(bool async)> notify_hist_clear;
        std::function<void(bool async, int index)> notify_hist_remove;
        
        HttpgdDataStore();
        ~HttpgdDataStore();

        int page_new(double width, double height);
        void page_put(int index, std::shared_ptr<dc::DrawCall> dc, bool silent);
        void page_clear(int index, bool silent);
        void page_remove(int index, bool silent);
        void page_remove_all();
        void page_fill(int index, int fill);
        void page_resize(int index, double w, double h);
        double page_get_width(int index);
        double page_get_height(int index);
        void page_clip(int index, double x0, double x1, double y0, double y1);
        int page_count();

        int get_upid() const;

        std::string api_state_json();
        std::string api_state_json(const HttpgdServerConfig *config, const std::string &host);
        std::string api_state_json(const HttpgdServerConfig *config, int port);
        std::string api_svg(bool async, int index, double width, double height);
        bool api_remove(bool async, int index);
        bool api_clear(bool async);

    private:

        std::vector<dc::Page> m_pages;
        std::mutex m_page_mutex;
        std::atomic<int> m_upid;
        void m_inc_upid();
        
        void m_build_svg(int index, std::string *buf);
        void m_build_state_json(std::string *buf);
    };

} // namespace httpgd

#endif // HTTPGD_SERVER_H