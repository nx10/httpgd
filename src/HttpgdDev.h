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

    HttpgdDev(pDevDesc dd, std::string host, int port, Rcpp::List aliases, double width, double height);
    ~HttpgdDev();

    void userResized();

  private:
  };

} // namespace httpgd

#endif