
#include <Rcpp.h>
#include <string>
#include "HttpgdDev.h"

namespace httpgd
{
    // returns system path to {package}/inst/www/{filename}
    std::string get_wwwpath(const std::string &filename)
    {
        Rcpp::Environment base("package:base");
        Rcpp::Function sys_file = base["system.file"];
        Rcpp::StringVector res = sys_file("www", filename,
                                          Rcpp::_["package"] = "httpgd");
        return std::string(res[0]);
    }

    std::string read_txt(const std::string &filepath)
    {
        std::ifstream t(get_wwwpath("index.html"));
        std::stringstream buffer;
        buffer << t.rdbuf();
        return std::string(buffer.str());
    }

    const int HISTORY_MAGIC = 8927;

    HttpgdDev::HttpgdDev(pDevDesc t_dd, const HttpgdDevStartParams &params)
        : dd(t_dd),
          server(params.host, params.port, params.width, params.height, params.recording, params.cors, params.use_token, params.token),
          history(std::string(".httpgdPlots_").append(std::to_string(params.port))),
          font(params.aliases), m_target(0), m_target_open(0)
    {
        // setup callbacks
        server.notify_hist_clear = [&]() {
            later::later([](void *ddp) {
                auto dd = static_cast<pDevDesc>(ddp);
                auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);

                xd->event_hist_clear();

                xd->server.replaying = false;
            },
                         dd, 0.0);
        };
        server.notify_replay = [&]() {
            if (server.replaying) // this should always be true (can't assert)
            {
                later::later([](void *ddp) {
                    auto dd = static_cast<pDevDesc>(ddp);
                    auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);

                    xd->render_page(xd->server.replaying_index + 1);

                    xd->server.replaying = false;
                },
                             dd, 0.0);
            }
        };

        // read live server html
        server.set_livehtml(read_txt(get_wwwpath("index.html")));
    }

    HttpgdDev::~HttpgdDev() = default;

    void HttpgdDev::put(std::shared_ptr<dc::DrawCall> dc)
    {
        if (m_target == 0)
            return;

        server.page_put(m_target - 1, dc);
    }

    void HttpgdDev::new_page(double width, double height, int fill)
    {
        // Rcpp::Rcout << "[new_page]\n";
        if (!server.replaying)
        {
            if (m_target_open > 0) // no previous pages
            {
                // Rcpp::Rcout << "    -> record open page in history\n";
                history.set_last(m_target_open - 1, dd);
            }
            // Rcpp::Rcout << "    -> add new page to server\n";
            m_target = server.page_new(width, height) + 1;
            m_target_open = m_target;
        }
        else
        {
            // Rcpp::Rcout << "    -> rewrite target: " << m_target << "\n";
            // Rcpp::Rcout << "    -> clear page\n";
            if (m_target != 0)
                server.page_clear(m_target - 1);
        }
        if (m_target != 0)
            server.page_fill(m_target - 1, fill);
    }
    void HttpgdDev::page_size(double *width, double *height)
    {
        unsigned int t = (m_target == 0) ? m_target_open : m_target;

        *width = server.page_get_width(t - 1);
        *height = server.page_get_height(t - 1);
    }
    void HttpgdDev::clip_page(double x0, double x1, double y0, double y1)
    {
        if (m_target == 0)
            return;
        server.page_clip(m_target - 1, x0, x1, y0, y1);
    }

    void HttpgdDev::render_page(unsigned int render_target)
    {
        // Rcpp::Rcout << "[render_page] render_target=" << render_target << "\n";

        if (render_target == 0)
            return;
        if (render_target == m_target_open)
        {
            // Rcpp::Rcout << "    -> open page. target_open="<< m_target << "\n";
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            GEplayDisplayList(desc2GEDesc(dd)); // replay active page
            m_target = m_target_open;           // set target to open page for new draw calls
        }
        else
        {
            // Rcpp::Rcout << "    -> old page. target_open="<< m_target_open << "\n";
            history.set_current(m_target_open - 1, dd);

            m_target = render_target;
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            history.play(m_target - 1, dd);
            m_target = 0;
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            history.play(m_target_open - 1, dd); // recreate previous state
            m_target = m_target_open;            // set target to open page for new draw calls
        }
    }

    void HttpgdDev::event_hist_clear()
    {
        history.clear();
        m_target = 0;
        m_target_open = 0;
    }
} // namespace httpgd