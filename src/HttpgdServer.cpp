
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
        : replaying(false),
          history_recording(t_recording),
          server_ready(false),
          m_host(t_host), m_port(t_port), m_svr_cors(t_cors),
          m_svr_use_token(t_use_token), m_svr_token(t_token),
          m_pages(), m_upid(0)
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
    unsigned int HttpgdServer::page_count()
    {
        unsigned int i;
        m_page_mutex.lock();
        i = static_cast<int>(m_pages.size());
        m_page_mutex.unlock();
        return i;
    }
    unsigned int HttpgdServer::page_new(double width, double height)
    {
        unsigned int i;
        m_page_mutex.lock();
        m_pages.push_back(dc::Page(width, height));
        i = static_cast<int>(m_pages.size()) - 1;
        m_page_mutex.unlock();
        return i;
    }
    void HttpgdServer::page_put(unsigned int index, std::shared_ptr<dc::DrawCall> dc)
    {
        m_page_mutex.lock();
        m_pages[index].put(dc);
        if (!replaying)
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_clear(unsigned int index)
    {
        m_page_mutex.lock();
        m_pages[index].clear();
        if (!replaying)
        {
            m_inc_upid();
        }
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_clear_all()
    {
        m_page_mutex.lock();
        for (auto p : m_pages)
        {
            p.clear();
        }
        m_pages.clear();
        m_inc_upid();
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_fill(unsigned int index, int fill)
    {
        m_page_mutex.lock();
        m_pages[index].fill = fill;
        m_page_mutex.unlock();
    }
    void HttpgdServer::build_svg(unsigned int index, std::string *buf)
    {
        m_page_mutex.lock();
        m_pages[index].build_svg(buf);
        m_page_mutex.unlock();
    }
    void HttpgdServer::page_resize(unsigned int index, double w, double h)
    {
        m_page_mutex.lock();
        m_pages[index].width = w;
        m_pages[index].height = h;
        m_pages[index].clear();
        m_page_mutex.unlock();
    }
    double HttpgdServer::page_get_width(unsigned int index)
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_pages[index].width;
        m_page_mutex.unlock();
        return d;
    }
    double HttpgdServer::page_get_height(unsigned int index)
    {
        double d = 0.0;
        m_page_mutex.lock();
        d = m_pages[index].height;
        m_page_mutex.unlock();
        return d;
    }
    void HttpgdServer::set_livehtml(const std::string &livehtml)
    {
        m_livehtml = livehtml;
    }
    void HttpgdServer::page_clip(unsigned int index, double x0, double x1, double y0, double y1)
    {
        m_page_mutex.lock();
        m_pages[index].clip(x0, x1, y0, y1);
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
        buf.append("\"upid\": ").append(std::to_string(m_upid));
        buf.append(", \"hrecording\": ").append(history_recording ? "true" : "false");
        m_page_mutex.lock();
        buf.append(", \"hsize\": ").append(std::to_string(m_pages.size()));
        m_page_mutex.unlock();
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
    unsigned int HttpgdServer::get_upid() const
    {
        return m_upid;
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

    // If the graphics engine scales any lower
    // it shows "figure margins too large"
    // and gets permanently in an invalid state.
    const double MIN_WIDTH = 200;
    const double MIN_HEIGHT = 200;

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
                unsigned int pcount = page_count();

                if (pcount == 0)
                {
                    res.set_content("<svg width=\"10\" height=\"10\" xmlns=\"http://www.w3.org/2000/svg\"></svg>", "image/svg+xml");
                }
                else
                {
                    unsigned int cli_index = pcount - 1;

                    if (req.has_param("index"))
                    {
                        std::string ptxt = req.get_param_value("index");
                        int pval = 0;
                        if (trystoi(ptxt, &pval))
                        {
                            unsigned int upval = pval;
                            if (upval >= 0 && upval < pcount)
                            {
                                cli_index = upval;
                            }
                        }
                    }

                    // get current state
                    m_page_mutex.lock();
                    double old_width = m_pages[cli_index].width;
                    double old_height = m_pages[cli_index].height;
                    m_page_mutex.unlock();

                    double cli_width = old_width;
                    double cli_height = old_height;

                    // get params
                    if (req.has_param("width"))
                    {
                        std::string ptxt = req.get_param_value("width");
                        double pval = 0.0;
                        if (trystod(ptxt, &pval))
                        {
                            if (pval < MIN_WIDTH)
                            {
                                pval = MIN_WIDTH;
                            }
                            cli_width = pval;
                        }
                    }
                    if (req.has_param("height"))
                    {
                        std::string ptxt = req.get_param_value("height");
                        double pval = 0.0;
                        if (trystod(ptxt, &pval))
                        {
                            if (pval < MIN_HEIGHT)
                            {
                                pval = MIN_HEIGHT;
                            }
                            cli_height = pval;
                        }
                    }

                    // Check if replay needed
                    if (std::abs(cli_width - old_width) > 0.1 ||
                        std::abs(cli_height - old_height) > 0.1)
                    {

                        m_page_mutex.lock();
                        m_pages[cli_index].width = cli_width;
                        m_pages[cli_index].height = cli_height;
                        m_pages[cli_index].clear(); // todo is this needed?
                        m_page_mutex.unlock();

                        while (replaying)
                        {
                        } // make sure we dont replay already
                        replaying_index = cli_index;
                        replaying = true;
                        notify_replay();
                        while (replaying)
                        {
                        } // block while replaying
                    }

                    std::string s = "";
                    s.reserve(1000000);
                    this->build_svg(cli_index, &s);
                    res.set_content(s, "image/svg+xml");
                }
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

                page_clear_all();

                while (replaying)
                {
                } // make sure we dont replay already
                replaying = true;
                notify_hist_clear();
                while (replaying)
                {
                } // block while replaying

                res.set_content(build_state_json(false), "application/json");
            }
        });

        if (m_port <= 0)
        {
            m_port = m_svr.bind_to_any_port(m_host.c_str());
        }
        else
        {
            m_svr.bind_to_port(m_host.c_str(), m_port);
        }
        server_ready = true;
        m_svr.listen_after_bind();
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