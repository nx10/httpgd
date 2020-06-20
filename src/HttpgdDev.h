#ifndef HTTPGD_DEV_H
#define HTTPGD_DEV_H

#include <Rcpp.h>
#include <R_ext/GraphicsDevice.h>
#include <R_ext/GraphicsEngine.h>
//#include "httplib.h"

#include <mutex>

#include "HttpgdServerConfig.h"
#include "HttpgdDataStore.h"
#include "HttpgdServerTask.h"

#include "PlotHistory.h"
#include "FontAnalyzer.h"

namespace httpgd
{
    class HttpgdServer;

    struct HttpgdDevStartParams
    {
        int bg;
        double width;
        double height;
        double pointsize;
        Rcpp::List &aliases;
    };

    class HttpgdDev
    {
    public:
        pDevDesc dd;
        FontAnalyzer font;
        
        std::atomic<bool> replaying;      // Is the device replaying
        std::atomic<int> replaying_index; // Index to replay
        std::atomic<double> replaying_width; // Index to replay
        std::atomic<double> replaying_height; // Index to replay

        HttpgdDev(pDevDesc t_dd, const HttpgdServerConfig &t_config, const HttpgdDevStartParams &t_params);
        ~HttpgdDev();

        void put(std::shared_ptr<dc::DrawCall> dc);

        void new_page(double width, double height, int fill);
        void page_size(double *width, double *height);
        void clip_page(double x0, double x1, double y0, double y1);
        void mode(bool start_draw);

        void render_page(int target, double width, double height);
        void hist_clear();
        void hist_remove(int target);
        
        void await_render_page(int target, double width, double height);
        void await_hist_clear();
        void await_hist_remove(int target);

        void start_server();
        void shutdown_server();
        int server_await_port();

        std::string store_state_json(bool include_config);
        std::string store_svg(int index, double width, double height);
        bool store_remove(int index);
        bool store_clear();
        int store_get_upid();
        int store_get_page_count();

        const HttpgdServerConfig *get_server_config();

    private:
        PlotHistory m_history;
        HttpgdDataStore m_data_store;
        const HttpgdServerConfig m_svr_config;
        HttpgdServerTask m_svr_task;

        int m_target; // current draw target. target = index + 1 (0 reserved for special case)
        int m_target_open; // open draw target. New draw calls from R always target this. target = index + 1 (0 reserved for special case)
    };

} // namespace httpgd

#endif