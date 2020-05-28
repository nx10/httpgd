#ifndef HTTPGD_SERVER_H
#define HTTPGD_SERVER_H

#include "httplib.h"

#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include "DrawData.h"

namespace httpgd
{

    class HttpgdServer
    {
    public:
        // callbacks
        std::function<void()> notify_resized;
        //std::function<void(bool)> notify_record;
        std::function<void()> notify_hist_play;
        std::function<void()> notify_hist_clear;

        HttpgdServer(const std::string &t_host, int t_port, double t_width, double t_height, bool t_recording);
        ~HttpgdServer();

        void start();
        void stop();

        void page_put(dc::DrawCall *dc);
        void page_clear();
        void page_fill(int fill);
        void page_resize(double w, double h);
        double page_get_width();
        double page_get_height();
        void page_clip(double x0, double x1, double y0, double y1);

        void build_svg(std::string *buf);
        std::string build_state_json(bool include_host);

        void set_livehtml(const std::string &livehtml);

        bool is_recording();
        void set_history_size(int history_size);
        int get_history_index();

    private:
        std::string m_host;
        int m_port;
        std::string m_livehtml;

        std::thread m_svr_thread;
        httplib::Server m_svr;

        dc::Page m_page;
        std::mutex m_page_mutex;

        bool m_history_recording;
        int m_history_index;
        int m_history_size;
        std::mutex m_history_mutex;

        void m_svr_main();
    };

    bool check_server_started(const std::string &host, int port);

} // namespace httpgd

#endif // HTTPGD_SERVER_H