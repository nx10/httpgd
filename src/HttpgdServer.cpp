
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
    HttpgdServer::HttpgdServer(const std::string &t_host, int t_port,
                               double t_width, double t_height,
                               bool t_recording,
                               bool t_cors, bool t_use_token, std::string t_token)
        : replaying(false), needsave(false),
          history_index(-1), history_size(0),
          history_recording(t_recording),
          m_host(t_host), m_port(t_port), m_svr_cors(t_cors),
          m_svr_use_token(t_use_token), m_svr_token(t_token),
          m_page(t_width, t_height), m_upid(0)
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
        if (!replaying)
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_clear()
    {
        m_page_mutex.lock();
        m_page.clear();
        if (!replaying)
        {
            m_inc_upid();
        }
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
            if (m_svr_use_token)
            {
                buf.append("\"token\": \"").append(m_svr_token);
                buf.append("\", ");
            }
        }
        m_page_mutex.lock();
        buf.append("\"upid\": ").append(std::to_string(m_upid));
        buf.append(", \"width\": ").append(std::to_string(m_page.width));
        buf.append(", \"height\": ").append(std::to_string(m_page.height));
        m_page_mutex.unlock();
        buf.append(", \"hrecording\": ").append(history_recording ? "true" : "false");
        buf.append(", \"hsize\": ").append(std::to_string(history_size));
        buf.append(", \"hindex\": ").append(std::to_string(history_index));
        buf.append(", \"needsave\": ").append(needsave ? "true" : "false");
        buf.append(" }");
        return buf;
    }

    inline bool trystod(const std::string &parse, double *out)
    {
        try
        {
            *out = std::stod(parse);
            return true;
        }
        catch (const std::exception &e)
        {
            return false;
        }
    }
    inline bool trystoi(const std::string &parse, int *out)
    {
        try
        {
            *out = std::stoi(parse);
            return true;
        }
        catch (const std::exception &e)
        {
            return false;
        }
    }

    bool HttpgdServer::last_page() const
    {
        return history_index == history_size - 1;
    }
    std::string HttpgdServer::get_host() const
    {
        return m_host;
    }
    int HttpgdServer::get_port() const
    {
        return m_port;
    }
    std::string HttpgdServer::get_token() const
    {
        return m_svr_use_token ? m_svr_token : "";
    }

    const std::string SVR_INJECT_KEYWORD = "/*SRVRPARAMS*/";
    const std::string SVR_403 = "Forbidden.";

    void HttpgdServer::m_inc_upid()
    {
        if (m_upid < 1000000)
        {
            m_upid += 1;
        }
        else
        {
            m_upid = 0;
        }
    }

    bool HttpgdServer::m_prepare_req(const httplib::Request &req, httplib::Response &res) const
    {
        if (m_svr_cors)
        {
            res.set_header("Access-Control-Allow-Origin", "*");
        }
        if (m_svr_use_token &&
            !(req.get_header_value("X-HTTPGD-TOKEN") == m_svr_token ||
              req.get_param_value("token") == m_svr_token))
        {
            res.set_content(SVR_403, "text/plain");
            res.status = 403;
            return false;
        }
        return true;
    }

    void HttpgdServer::m_svr_main()
    {
        using httplib::Params;
        using httplib::Request;
        using httplib::Response;
        using httplib::detail::parse_query_text;

        m_svr.Get("/", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                res.set_content("httpgd server running.", "text/plain");
            }
        });
        m_svr.Get("/live", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                // build params
                std::string sparams = build_state_json(true);
                sparams.append("/*");

                // inject params
                std::string html(m_livehtml);
                size_t start_pos = m_livehtml.find(SVR_INJECT_KEYWORD);
                if (start_pos != std::string::npos)
                {
                    html.replace(start_pos, sizeof(SVR_INJECT_KEYWORD) - 1, sparams);
                }

                res.set_content(html, "text/html");
            }
        });
        m_svr.Get("/svg", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                // get current state
                m_page_mutex.lock();
                double old_width = m_page.width;
                double old_height = m_page.height;
                m_page_mutex.unlock();
                int old_index = history_index;

                double cli_width = old_width;
                double cli_height = old_height;
                int cli_index = old_index;

                // get params
                if (req.has_param("width"))
                {
                    std::string ptxt = req.get_param_value("width");
                    double pval = 0.0;
                    if (trystod(ptxt, &pval) && pval > 0)
                    {
                        cli_width = pval;
                    }
                }
                if (req.has_param("height"))
                {
                    std::string ptxt = req.get_param_value("height");
                    double pval = 0.0;
                    if (trystod(ptxt, &pval) && pval > 0)
                    {
                        cli_height = pval;
                    }
                }
                if (req.has_param("index"))
                {
                    std::string ptxt = req.get_param_value("index");
                    int pval = 0;
                    if (trystoi(ptxt, &pval))
                    {
                        if (pval == -1 || pval < 0 || pval >= history_size)
                        {
                            cli_index = history_size - 1;
                        }
                        else
                        {
                            cli_index = pval;
                        }
                    }
                }

                // Check if replay needed
                if (std::abs(cli_width - old_width) > 0.01 ||
                    std::abs(cli_height - old_height) > 0.01 ||
                    cli_index != old_index)
                {
                    m_page_mutex.lock();
                    m_page.width = cli_width;
                    m_page.height = cli_height;
                    m_page.clear();
                    m_page_mutex.unlock();
                    history_index = cli_index; // todo

                    replaying = true;
                    notify_replay();
                    while (replaying)
                    {
                    } // block while replaying
                }

                std::string s = "";
                s.reserve(1000000);
                this->build_svg(&s);
                res.set_content(s, "image/svg+xml");
            }
        });
        m_svr.Get("/state", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                res.set_content(build_state_json(false), "application/json");
            }
        });
        m_svr.Get("/clear", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {

                page_clear();
                replaying = true;
                notify_hist_clear();
                while (replaying)
                {
                } // block while replaying

                res.set_content(build_state_json(false), "application/json");
            }
        });

        m_svr.listen(m_host.c_str(), m_port);
    }

    const time_t CHECK_OCCUPIED_TIMEOUT = 300000;
    bool check_server_started(const std::string &host, int port)
    {
        httplib::Client cli(host, port);
        cli.set_connection_timeout(0, CHECK_OCCUPIED_TIMEOUT); // 300 milliseconds
        auto res = cli.Get("/");
        if (res)
        {
            return true;
        }
        return false;
    }

} // namespace httpgd