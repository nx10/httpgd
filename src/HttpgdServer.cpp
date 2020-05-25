
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
  HttpgdServer::HttpgdServer(std::string host, int port, double width, double height, std::function<void()> userResized)
      : m_host(host), m_port(port), m_page(width, height), m_userResized(userResized)
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
      std::string sparams = std::string("{ host: \"").append(m_host);
      sparams.append("\", port: ").append(std::to_string(m_port));
      m_page_mutex.lock();
      sparams.append(", width: ").append(std::to_string(m_page.m_width));
      sparams.append(", height: ").append(std::to_string(m_page.m_height));
      m_page_mutex.unlock();
      sparams.append(" }/*");

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
    m_svr.Get("/check", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      std::string buffer;
      buffer.reserve(200);
      m_page_mutex.lock();
      buffer.append("{ \"upid\": ").append(std::to_string(m_page.get_upid()));
      buffer.append(", \"width\": ").append(std::to_string(m_page.m_width));
      buffer.append(", \"height\": ").append(std::to_string(m_page.m_height));
      buffer.append(" }");
      m_page_mutex.unlock();

      res.set_content(buffer, "application/json");
    });
    m_svr.Post("/rs", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      Params par;
      detail::parse_query_text(req.body, par);

      double w = m_page.m_width;
      double h = m_page.m_height;

      for (const auto &e : par)
      {
        if (e.first == "w")
        {
          try
          {
            w = std::stod(e.second);
          }
          catch (const std::exception &e)
          {
          }
        }
        else if (e.first == "h")
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

        m_userResized();
        //m_dd->size(&(m_dd->left), &(m_dd->right), &(m_dd->bottom), &(m_dd->top), m_dd);
        //later::later([](void *dd) { GEplayDisplayList(desc2GEDesc((pDevDesc)dd)); }, m_dd, 0.0);
      }

      res.set_content("{ \"status\": \"ok\" }", "application/json");
    });

    //m_svr.Get("/stop",
    //        [&](const Request & /*req*/, Response & /*res*/) { m_svr.stop(); });

    m_svr.listen(m_host.c_str(), m_port);
  }

} // namespace httpgd