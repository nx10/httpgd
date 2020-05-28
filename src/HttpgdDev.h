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

    class HttpgdDev
    {
    public:
        pDevDesc dd;
        HttpgdServer server;
        PlotHistory history;
        FontAnalyzer font;

        HttpgdDev(pDevDesc t_dd, const std::string &t_host, int t_port,
                  const Rcpp::List &t_aliases,
                  double t_width, double t_height, bool t_recording, bool t_cors);
        ~HttpgdDev();

        void hist_new_page();
        void hist_update_size();

        void event_resized();
        //void user_hist_record(bool recording);
        void event_hist_play();
        void event_hist_clear();

    private:
        bool m_replaying; // Is the device replaying
        bool m_needsave;  // Should a snapshot be saved when the plot changes
                          // see also m_recording in HttpgdServer
    };

} // namespace httpgd

#endif