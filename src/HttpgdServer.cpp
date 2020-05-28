
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "httplib.h"

#include <string>
#include <vector>
#include <mutex>

#include "DrawData.h"
#include "HttpgdServer.h"

// Do not include any R headers here!

namespace httpgd
{
    HttpgdServer::HttpgdServer(const std::string &t_host, int t_port, double t_width, double t_height, bool t_recording)
        : m_host(t_host), m_port(t_port),
          m_page(t_width, t_height), m_history_recording(t_recording),
          m_history_index(-1), m_history_size(0)
    {
    }
    HttpgdServer::~HttpgdServer()
    {
        stop();
    }
    void HttpgdServer::start()
    {
        m_svr_thread = std::thread(&HttpgdServer::m_svr_main, this);
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
        m_page.fill = fill;
        m_page_mutex.unlock();
    }
    void HttpgdServer::build_svg(std::string *buf)
    {
        m_page_mutex.lock();
        m_page.build_svg(buf);
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_resize(double w, double h)
    {
        m_page_mutex.lock();
        m_page.width = w;
        m_page.height = h;
        m_page.clear();
        m_page_mutex.unlock();
    }
    double HttpgdServer::page_get_width()
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_page.width;
        m_page_mutex.unlock();
        return d;
    }
    double HttpgdServer::page_get_height()
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_page.height;
        m_page_mutex.unlock();
        return d;
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
        bool rec = false;
        m_history_mutex.lock();
        rec = m_history_recording;
        m_history_mutex.unlock();
        return rec;
    }
    void HttpgdServer::set_history_size(int history_size)
    {
        m_history_mutex.lock();
        if (m_history_size != history_size)
        {
            m_history_size = history_size;
            m_history_index = m_history_size - 1; // jump to newest
        }
        m_history_mutex.unlock();
    }
    int HttpgdServer::get_history_index()
    {
        int i = 0;
        m_history_mutex.lock();
        i = m_history_index;
        m_history_mutex.unlock();
        return i;
    }

    std::string HttpgdServer::build_state_json(bool include_host)
    {
        std::string buf;
        buf.reserve(200);
        buf.append("{ ");
        if (include_host)
        {
            buf.append("\"host\": \"").append(m_host);
            buf.append("\", \"port\": ").append(std::to_string(m_port));
            buf.append(", ");
        }
        m_page_mutex.lock();
        buf.append("\"upid\": ").append(std::to_string(m_page.get_upid()));
        buf.append(", \"width\": ").append(std::to_string(m_page.width));
        buf.append(", \"height\": ").append(std::to_string(m_page.height));
        m_page_mutex.unlock();
        m_history_mutex.lock();
        buf.append(", \"recording\": ").append(m_history_recording ? "true" : "false");
        buf.append(", \"hsize\": ").append(std::to_string(m_history_size));
        buf.append(", \"hindex\": ").append(std::to_string(m_history_index));
        m_history_mutex.unlock();
        buf.append(" }");
        return buf;
    }

    inline void trystod(const std::string &parse, double *out)
    {
        try
        {
            *out = std::stod(parse);
        }
        catch (const std::exception &e)
        {
        }
    }

    void HttpgdServer::m_svr_main()
    {
        using httplib::Params;
        using httplib::Request;
        using httplib::Response;
        using httplib::detail::parse_query_text;

        m_svr.Get("/", [](const Request & /*req*/, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content("httpgd server running.", "text/plain");
        });
        m_svr.Get("/live", [this](const Request & /*req*/, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");

            // build params
            std::string sparams = build_state_json(true);
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
            std::string s = "";
            s.reserve(1000000);
            this->build_svg(&s);
            res.set_content(s, "image/svg+xml");
        });
        m_svr.Get("/state", [this](const Request & /*req*/, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(build_state_json(false), "application/json");
        });
        m_svr.Post("/resize", [this](const Request &req, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            Params par;
            parse_query_text(req.body, par);

            double user_w = -1;
            double user_h = -1;

            for (const auto &e : par)
            {
                if (e.first == "width")
                {
                    trystod(e.second, &user_w);
                }
                else if (e.first == "height")
                {
                    trystod(e.second, &user_h);
                }
            }
            
            m_page_mutex.lock();
            double old_w = m_page.width;
            double old_h = m_page.height; // todo mutex lock
            m_page_mutex.unlock();

            double new_w = (user_w > 0) && (user_w != old_w) ? user_w : old_w;
            double new_h = (user_h > 0) && (user_h != old_h) ? user_h : old_h;

            if (new_w != old_w || new_h != old_h)
            {
                page_resize(new_w, new_h);
                notify_resized();
            }

            res.set_content(build_state_json(false), "application/json");
        });
        m_svr.Post("/next", [this](const Request &req, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");

            if (m_history_index < m_history_size - 1)
            {
                m_history_index += 1;
                notify_hist_play();
            }

            res.set_content(build_state_json(false), "application/json");
        });
        m_svr.Post("/prev", [this](const Request &req, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");

            if (m_history_index > 0)
            {
                m_history_index -= 1;
                notify_hist_play();
            }

            res.set_content(build_state_json(false), "application/json");
        });
        m_svr.Post("/clear", [this](const Request &req, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");

            page_clear();
            notify_hist_clear();

            res.set_content(build_state_json(false), "application/json");
        });
        m_svr.Post("/record", [this](const Request &req, Response &res) {
            res.set_header("Access-Control-Allow-Origin", "*");

            Params par;
            parse_query_text(req.body, par);

            for (const auto &e : par)
            {
                if (e.first == "recording")
                {
                    if (e.second == "true")
                    {
                        m_history_recording = true;
                    }
                    else if (e.second == "false")
                    {
                        m_history_recording = false;
                    }
                    break;
                }
            }

            res.set_content(build_state_json(false), "application/json");
        });

        //m_svr.Get("/stop",
        //        [&](const Request & /*req*/, Response & /*res*/) { m_svr.stop(); });

        m_svr.listen(m_host.c_str(), m_port);
    }

    const time_t CHECK_OCCUPIED_TIMEOUT = 300000;
    bool check_server_started(const std::string &host, int port)
    {
        httplib::Client cli(host, port);
        cli.set_connection_timeout(0, CHECK_OCCUPIED_TIMEOUT); // 300 milliseconds
        auto res = cli.Get("/");
        if (res && res->status == 200)
        {
            return true;
        }
        return false;
    }

} // namespace httpgd