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
        std::function<void()> notify_replay;
        //std::function<void()> notify_resized;
        //std::function<void(bool)> notify_record;
        //std::function<void()> notify_hist_play;
        std::function<void()> notify_hist_clear;
        
        std::atomic<bool> replaying; // Is the device replaying
        std::atomic<bool> needsave;  // Should a snapshot be saved when the plot changes
        std::atomic<int> history_index; // last draw history index
        std::atomic<int> history_size; // last draw history index
        std::atomic<bool> history_recording; // should new pages be added to plot history

        HttpgdServer(const std::string &t_host, int t_port,
                     double t_width, double t_height,
                     bool t_recording,
                     bool t_cors, bool t_use_token, std::string t_token);
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

        bool last_page() const;
        std::string get_host() const;
        int get_port() const;
        std::string get_token() const;

    private:
        std::string m_host;
        int m_port;
        std::string m_livehtml;

        std::thread m_svr_thread;
        httplib::Server m_svr;
        bool m_svr_cors;
        bool m_svr_use_token;
        std::string m_svr_token;

        dc::Page m_page;
        std::mutex m_page_mutex;

        bool prepare_req(const httplib::Request &req, httplib::Response &res) const;
        void m_svr_main();
    };

    bool check_server_started(const std::string &host, int port);

} // namespace httpgd

#endif // HTTPGD_SERVER_H