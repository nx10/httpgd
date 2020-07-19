#ifndef HTTPGD_DEV_H
#define HTTPGD_DEV_H

#include <Rcpp.h>

#include <mutex>
#include <memory>

#include "devGeneric.h"
#include "HttpgdServerConfig.h"
#include "HttpgdDataStore.h"
#include "HttpgdHttpTask.h"

#include "PlotHistory.h"

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

    class HttpgdDev : public devGeneric
    {
    public:

        // Font handling
        Rcpp::List system_aliases;
        Rcpp::List user_aliases;
        
        std::atomic<bool> replaying{false};      // Is the device replaying
        std::atomic<int> replaying_index{0}; // Index to replay
        std::atomic<double> replaying_width{0}; // Index to replay
        std::atomic<double> replaying_height{0}; // Index to replay

        HttpgdDev(const HttpgdServerConfig &t_config, const HttpgdDevStartParams &t_params);
        virtual ~HttpgdDev();

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

        std::shared_ptr<HttpgdServerConfig> get_server_config();
        
        static std::string random_token(int len);

        // set device size
        void resize_device_to_page(pDevDesc dd);

    protected:
        virtual void dev_activate(pDevDesc dd);
        virtual void dev_deactivate(pDevDesc dd);
        virtual void dev_close(pDevDesc dd);
        virtual void dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd);
        virtual void dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd);
        virtual void dev_newPage(pGEcontext gc, pDevDesc dd);
        virtual void dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd);
        virtual void dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd);
        virtual double dev_strWidth(const char *str, pGEcontext gc, pDevDesc dd);
        virtual void dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd);
        virtual void dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd);
        virtual void dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd);
        virtual void dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd);
        virtual void dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd);
        virtual void dev_mode(int mode, pDevDesc dd);
        virtual void dev_metricInfo(int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd);
        virtual void dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd);

    private:
        PlotHistory m_history;
        std::shared_ptr<HttpgdServerConfig> m_svr_config;
        std::shared_ptr<HttpgdDataStore> m_data_store;
        std::shared_ptr<http::HttpgdHttpTask> m_svr_task;

        int m_target{-1}; // current draw target. target = index + 1 (0 reserved for special case)
        int m_target_open{-1}; // open draw target. New draw calls from R always target this. target = index + 1 (0 reserved for special case)
        
        void put(std::shared_ptr<dc::DrawCall> dc);
    };

} // namespace httpgd

#endif