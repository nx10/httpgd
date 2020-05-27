#ifndef HTTPGD_DEV_H
#define HTTPGD_DEV_H

#include <Rcpp.h>
#include <R_ext/GraphicsDevice.h>
#include <R_ext/GraphicsEngine.h>
#include <later_api.h>
#include <gdtools.h>
#include "httplib.h"

#include <mutex>

#include "HttpgdServer.h"
#include "PlotHistory.h"

namespace httpgd
{
  class HttpgdServer;

  class HttpgdDev
  {
  public:
    pDevDesc m_dd;
    Rcpp::List m_system_aliases;
    Rcpp::List m_user_aliases;
    XPtrCairoContext m_cc;
    HttpgdServer m_server;
    PlotHistory m_history;

    bool m_replaying; // Is the device replaying
    bool m_needsave; // Should a snapshot be saved when the plot changes
    // see also m_recording in HttpgdServer

    HttpgdDev(pDevDesc dd, std::string host, int port, Rcpp::List aliases, double width, double height, bool recording);
    ~HttpgdDev();

    void user_resized();
    //void user_hist_record(bool recording);
    void user_hist_play();
    void user_hist_clear();

  private:
  };

} // namespace httpgd

#endif