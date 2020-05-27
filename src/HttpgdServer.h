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
    // callbacks
    std::function<void()> m_user_resized;
    //std::function<void(bool)> m_user_hist_record;
    std::function<void()> m_user_hist_play;
    std::function<void()> m_user_hist_clear;

    HttpgdServer(std::string host, int port, double width, double height, bool recording);
    ~HttpgdServer();

    void start();
    void stop();

    void page_put(dc::DrawCall *dc);
    void page_clear();
    void page_fill(int fill);
    void page_resize(double w, double h);
    double page_width();
    double page_height();
    void page_clip(double x0, double x1, double y0, double y1);
    
    void get_svg(std::string &buf);
    std::string get_state_json(bool include_host);

    void set_livehtml(const std::string &livehtml);

    bool is_recording();
    void set_history_size(int history_size);
    int get_history_index();

  private:
    std::string m_host;
    int m_port;
    std::string m_livehtml;

    std::thread m_svr_thread;
    httplib::Server m_svr;

    dc::Page m_page;
    std::mutex m_page_mutex;

    bool m_recording; // Is the device recording
    int m_history_index;
    int m_history_size;
    

    void svr_main();
  };

} // namespace httpgd

#endif // HTTPGD_SERVER_H