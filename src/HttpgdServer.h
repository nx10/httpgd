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
        std::atomic<unsigned int> replaying_index; // Index to replay
        //std::atomic<bool> needsave;  // Should a snapshot be saved when the plot changes
        //std::atomic<int> history_index; // last draw history index
        std::atomic<bool> history_recording; // should new pages be added to plot history
        
        std::atomic<bool> server_ready; // server is bound to a port

        HttpgdServer(const std::string &t_host, int t_port,
                     double t_width, double t_height,
                     bool t_recording,
                     bool t_cors, bool t_use_token, std::string t_token);
        ~HttpgdServer();

        void start();
        void stop();

        unsigned int page_new(double width, double height);
        void page_put(unsigned int index, std::shared_ptr<dc::DrawCall> dc);
        void page_clear(unsigned int index);
        void page_clear_all();
        void page_fill(unsigned int index, int fill);
        void page_resize(unsigned int index, double w, double h);
        double page_get_width(unsigned int index);
        double page_get_height(unsigned int index);
        void page_clip(unsigned int index, double x0, double x1, double y0, double y1);
        unsigned int page_count();

        void build_svg(unsigned int index, std::string *buf);
        std::string build_state_json(bool include_host);

        void set_livehtml(const std::string &livehtml);

        //bool last_page() const;
        std::string get_host() const;
        int get_port() const;
        std::string get_token() const;
        unsigned int get_upid() const;


    private:
        std::string m_host;
        int m_port;
        std::string m_livehtml;

        std::thread m_svr_thread;
        httplib::Server m_svr;
        bool m_svr_cors;
        bool m_svr_use_token;
        std::string m_svr_token;

        std::vector<dc::Page> m_pages;
        std::mutex m_page_mutex;
        std::atomic<unsigned int> m_upid;
        void m_inc_upid();

        bool m_prepare_req(const httplib::Request &req, httplib::Response &res) const;
        void m_svr_main();
    };

    bool check_server_started(const std::string &host, int port);

} // namespace httpgd

#endif // HTTPGD_SERVER_H