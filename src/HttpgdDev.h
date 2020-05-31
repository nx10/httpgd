#ifndef HTTPGD_DEV_H
#define HTTPGD_DEV_H

#include <Rcpp.h>
#include <R_ext/GraphicsDevice.h>
#include <R_ext/GraphicsEngine.h>
#include <later_api.h>
#include "httplib.h"

#include <mutex>

#include "HttpgdServer.h"
#include "PlotHistory.h"
#include "FontAnalyzer.h"

namespace httpgd
{
    class HttpgdServer;

    struct HttpgdDevStartParams
    {
        std::string host;
        int port;
        int bg;
        double width;
        double height;
        double pointsize;
        Rcpp::List &aliases;
        bool recording;
        bool cors;
        bool use_token;
        std::string token;
    };

    class HttpgdDev
    {
    public:
        pDevDesc dd;
        HttpgdServer server;
        PlotHistory history;
        FontAnalyzer font;

        HttpgdDev(pDevDesc t_dd, const HttpgdDevStartParams &params);
        ~HttpgdDev();

        void put(std::shared_ptr<dc::DrawCall> dc);

        void new_page(double width, double height, int fill);
        void page_size(double *width, double *height);
        void render_page(unsigned int target);
        void clip_page(double x0, double x1, double y0, double y1);

        void event_hist_clear();

    private:
        unsigned int m_target; // current draw target. target = index + 1 (0 reserved for special case)
        unsigned int m_target_open; // open draw target. New draw calls from R always target this. target = index + 1 (0 reserved for special case)
    };

} // namespace httpgd

#endif