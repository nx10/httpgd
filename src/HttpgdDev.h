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

        void hist_new_page();
        void hist_update_size();

        void event_hist_clear();

    private:
                          // see also m_recording in HttpgdServer
    };

} // namespace httpgd

#endif