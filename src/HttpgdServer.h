#ifndef HTTPGD_SERVER_H
#define HTTPGD_SERVER_H

#include "httplib.h"

#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include "drawdata.h"

namespace httpgd
{

  class HttpgdServer
  {
  public:
    HttpgdServer(std::string host, int port, double width, double height, std::function<void()> userResized);
    ~HttpgdServer();

    void start();
    void stop();
    void page_put(dc::DrawCall *dc);
    void page_clear();
    void page_fill(int fill);
    void get_svg(std::string &buf);
    void page_resize(double w, double h);
    double page_width();
    double page_height();
    void page_clip(double x0, double x1, double y0, double y1);

    void set_livehtml(const std::string &livehtml);

  private:
    std::string m_host;
    int m_port;
    std::string m_livehtml;

    std::thread m_svr_thread;
    httplib::Server m_svr;

    dc::Page m_page;
    std::mutex m_page_mutex;

    // callbacks
    std::function<void()> m_userResized;

    void svr_main();
  };

} // namespace httpgd

#endif // HTTPGD_SERVER_H