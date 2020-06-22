#include "HttpgdServerTask.h"

namespace httpgd
{

    const std::string SVR_INJECT_KEYWORD = "/*SRVRPARAMS*/";
    const std::string SVR_401 = "Unauthorized.";
    const std::string SVR_404 = "Not found.";

    HttpgdServerTask::HttpgdServerTask(HttpgdDataStore *t_data,
                                       const HttpgdServerConfig *t_config)
        : m_is_bound_to_port(false),
          m_bound_port(-1),
          m_config(t_config),
          m_data(t_data),
          m_svr()
    {
    }

    void HttpgdServerTask::stop()
    {
        m_svr.stop();
    }

    void HttpgdServerTask::execute()
    {
        m_run_server();
    }
    void HttpgdServerTask::complete()
    {
        //Rprintf("Background task ended.");
    }

    int HttpgdServerTask::await_port()
    {
        while (!m_is_bound_to_port)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
        return m_bound_port;
    }

    bool HttpgdServerTask::m_prepare_req(const httplib::Request &req, httplib::Response &res) const
    {
        if (m_config->cors)
        {
            res.set_header("Access-Control-Allow-Origin", "*");
        }
        if (m_config->use_token &&
            !(req.get_header_value("X-HTTPGD-TOKEN") == m_config->token ||
              req.get_param_value("token") == m_config->token))
        {
            res.set_content(SVR_401, "text/plain");
            res.status = 401;
            return false;
        }
        return true;
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

    void HttpgdServerTask::m_run_server()
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
                std::string sparams = m_data->api_state_json(m_config, req.get_header_value("Host"));
                sparams.append("/*");

                // inject params
                std::string html(m_config->livehtml);
                size_t start_pos = html.find(SVR_INJECT_KEYWORD);
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
                int cli_index = -1;
                double cli_width = -1;
                double cli_height = -1;

                if (req.has_param("index"))
                {
                    std::string ptxt = req.get_param_value("index");
                    trystoi(ptxt, &cli_index);
                }
                if (req.has_param("width"))
                {
                    std::string ptxt = req.get_param_value("width");
                    trystod(ptxt, &cli_width);
                }
                if (req.has_param("height"))
                {
                    std::string ptxt = req.get_param_value("height");
                    trystod(ptxt, &cli_height);
                }

                res.set_content(m_data->api_svg(true, cli_index, cli_width, cli_height), "image/svg+xml");
            }
        });
        m_svr.Get("/state", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                res.set_content(m_data->api_state_json(), "application/json");
            }
        });
        m_svr.Get("/clear", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                m_data->api_clear(true);
                res.set_content(m_data->api_state_json(), "application/json");
            }
        });
        m_svr.Get("/remove", [&](const Request &req, Response &res) {
            if (m_prepare_req(req, res))
            {
                int cli_index = -1;
                if (req.has_param("index"))
                {
                    std::string ptxt = req.get_param_value("index");
                    trystoi(ptxt, &cli_index);
                }

                if (!m_data->api_remove(true, cli_index))
                {
                    res.set_content(SVR_404, "text/plain");
                    res.status = 404;
                }
                else
                {
                    res.set_content(m_data->api_state_json(), "application/json");
                }
            }
        });

        if (m_config->port <= 0)
        {
            m_bound_port = m_svr.bind_to_any_port(m_config->host.c_str());
        }
        else
        {
            m_bound_port = m_config->port;
            m_svr.bind_to_port(m_config->host.c_str(), m_config->port);
        }
        m_is_bound_to_port = true;
        m_svr.listen_after_bind();
    } // namespace httpgd

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