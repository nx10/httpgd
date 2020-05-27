
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "httplib.h"

#include <string>
#include <vector>
#include <mutex>

#include "drawdata.h"

#include "HttpgdServer.h"

namespace httpgd
{
  HttpgdServer::HttpgdServer(std::string host, int port, double width, double height, bool recording)
      : m_host(host), m_port(port),
        m_page(width, height), m_recording(recording),
        m_history_index(-1), m_history_size(0)
  {
  }
  HttpgdServer::~HttpgdServer()
  {
    stop();
  }
  void HttpgdServer::start()
  {
    m_svr_thread = std::thread(&HttpgdServer::svr_main, this);
  }
  void HttpgdServer::stop()
  {
    m_svr.stop();
    if (m_svr_thread.joinable())
    {
      m_svr_thread.join();
    }
  }
  void HttpgdServer::page_put(dc::DrawCall *dc)
  {
    m_page_mutex.lock();
    m_page.put(dc);
    m_page_mutex.unlock();
  }
  void HttpgdServer::page_clear()
  {
    m_page_mutex.lock();
    m_page.clear();
    m_page_mutex.unlock();
  }
  void HttpgdServer::page_fill(int fill)
  {
    m_page_mutex.lock();
    m_page.m_fill = fill;
    m_page_mutex.unlock();
  }
  void HttpgdServer::get_svg(std::string &buf)
  {
    m_page_mutex.lock();
    m_page.to_svg(buf);
    m_page_mutex.unlock();
  }
  void HttpgdServer::page_resize(double w, double h)
  {
    m_page_mutex.lock();
    m_page.m_width = w;
    m_page.m_height = h;
    m_page.clear();
    m_page_mutex.unlock();
  }
  double HttpgdServer::page_width()
  {
    return m_page.m_width;
  }
  double HttpgdServer::page_height()
  {
    return m_page.m_height;
  }
  void HttpgdServer::set_livehtml(const std::string &livehtml)
  {
    m_livehtml = livehtml;
  }
  void HttpgdServer::page_clip(double x0, double x1, double y0, double y1)
  {
    m_page_mutex.lock();
    m_page.clip(x0, x1, y0, y1);
    m_page_mutex.unlock();
  }
  bool HttpgdServer::is_recording()
  {
    bool rec;
    m_page_mutex.lock();
    rec = m_recording;
    m_page_mutex.unlock();
    return rec;
  }
  void HttpgdServer::set_history_size(int history_size)
  {
    m_page_mutex.lock();
    if (m_history_size != history_size)
    {
      m_history_size = history_size;
      m_history_index = m_history_size - 1; // jump to newest
    }
    m_page_mutex.unlock();
  }
  int HttpgdServer::get_history_index()
  {
    int i;
    m_page_mutex.lock();
    i = m_history_index;
    m_page_mutex.unlock();
    return i;
  }

  std::string HttpgdServer::get_state_json(bool include_host)
  {
    std::string buf;
    buf.reserve(200);
    m_page_mutex.lock();
    buf.append("{ ");
    if (include_host)
    {
      buf.append("\"host\": \"").append(m_host);
      buf.append("\", \"port\": ").append(std::to_string(m_port));
      buf.append(", ");
    }
    buf.append("\"upid\": ").append(std::to_string(m_page.get_upid()));
    buf.append(", \"width\": ").append(std::to_string(m_page.m_width));
    buf.append(", \"height\": ").append(std::to_string(m_page.m_height));
    buf.append(", \"recording\": ").append(m_recording ? "true" : "false");
    buf.append(", \"hsize\": ").append(std::to_string(m_history_size));
    buf.append(", \"hindex\": ").append(std::to_string(m_history_index));
    buf.append(" }");
    m_page_mutex.unlock();
    return buf;
  }

  void HttpgdServer::svr_main()
  {
    using namespace httplib;

    m_svr.Get("/", [](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content("httpgd server running.", "text/plain");
    });
    m_svr.Get("/live", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      // build params
      std::string sparams = get_state_json(true);
      sparams.append("/*");

      // inject params
      std::string html(m_livehtml);
      size_t start_pos = m_livehtml.find("/*SRVRPARAMS*/");
      if (start_pos != std::string::npos)
      {
        html.replace(start_pos, sizeof("/*SRVRPARAMS*/") - 1, sparams);
      }

      res.set_content(html, "text/html");
    });
    m_svr.Get("/svg", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      std::string s;
      s.reserve(1000000);
      this->get_svg(s);
      res.set_content(s, "image/svg+xml");
    });
    m_svr.Get("/state", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(get_state_json(false), "application/json");
    });
    m_svr.Post("/resize", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      Params par;
      detail::parse_query_text(req.body, par);

      double w = m_page.m_width;
      double h = m_page.m_height; // todo mutex lock

      for (const auto &e : par)
      {
        if (e.first == "width")
        {
          try
          {
            w = std::stod(e.second);
          }
          catch (const std::exception &e)
          {
          }
        }
        else if (e.first == "height")
        {
          try
          {
            h = std::stod(e.second);
          }
          catch (const std::exception &e)
          {
          }
        }
      }

      if (w != m_page.m_width || h != m_page.m_height)
      {
        page_resize(w, h);

        m_user_resized();
        //m_dd->size(&(m_dd->left), &(m_dd->right), &(m_dd->bottom), &(m_dd->top), m_dd);
        //later::later([](void *dd) { GEplayDisplayList(desc2GEDesc((pDevDesc)dd)); }, m_dd, 0.0);
      }

      res.set_content(get_state_json(false), "application/json");
    });
    m_svr.Post("/next", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      if (m_history_index < m_history_size - 1)
      {
        m_history_index += 1;
        m_user_hist_play();
      }

      res.set_content(get_state_json(false), "application/json");
    });
    m_svr.Post("/prev", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      if (m_history_index > 0)
      {
        m_history_index -= 1;
        m_user_hist_play();
      }

      res.set_content(get_state_json(false), "application/json");
    });
    m_svr.Post("/clear", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      page_clear();
      m_user_hist_clear();

      res.set_content(get_state_json(false), "application/json");
    });
    m_svr.Post("/record", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      Params par;
      detail::parse_query_text(req.body, par);

      for (const auto &e : par)
      {
        if (e.first == "recording")
        {
          if (e.second == "true")
          {
            m_recording = true;
          }
          else if (e.second == "false")
          {
            m_recording = false;
          }
          break;
        }
      }

      res.set_content(get_state_json(false), "application/json");
    });

    //m_svr.Get("/stop",
    //        [&](const Request & /*req*/, Response & /*res*/) { m_svr.stop(); });

    m_svr.listen(m_host.c_str(), m_port);
  }

  bool check_server_started(std::string host, int port) {
    httplib::Client cli(host, port);
    cli.set_connection_timeout(0, 300000); // 300 milliseconds
    auto res = cli.Get("/");
    if (res && res->status == 200) {
      return true;
    }
    return false;
  }

} // namespace httpgd