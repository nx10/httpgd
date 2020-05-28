
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

    const int HISTORY_MAGIC = 8927;

    HttpgdDev::HttpgdDev(pDevDesc t_dd, const std::string &t_host, int t_port, const Rcpp::List &t_aliases, double t_width, double t_height, bool t_recording, bool t_cors)
        : dd(t_dd),
          server(t_host, t_port, t_width, t_height, t_recording, t_cors),
          history(HISTORY_MAGIC, 4, std::string(".httpgdPlots_").append(std::to_string(t_port))),
          font(t_aliases),
          m_replaying(false), m_needsave(false)
    {
        // setup callbacks
        server.notify_resized = [&]() { event_resized(); };
        //server.m_user_hist_record = [&](bool recording){ user_hist_record(recording); };
        server.notify_hist_play = [&]() { event_hist_play(); };
        server.notify_hist_clear = [&]() { event_hist_clear(); };

        // read live server html
        std::ifstream t(get_wwwpath("index.html"));
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string livehtml = std::string(buffer.str());
        server.set_livehtml(livehtml);

        // set start fill
        server.page_fill(dd->startfill);
    }

    HttpgdDev::~HttpgdDev() = default;

    void HttpgdDev::hist_new_page()
    {
        if (server.is_recording() && m_needsave)
        {
            history.push_last(dd);
        }
        m_needsave = !m_replaying;
        if (!m_replaying)
        {
            hist_update_size();
        }
    }
    void HttpgdDev::hist_update_size()
    {
        int history_size = history.size();
        if (m_needsave)
        {
            history_size += 1;
        }
        server.set_history_size(history_size);
    }

    void HttpgdDev::event_resized()
    {
        m_replaying = true;
        later::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            GEplayDisplayList(desc2GEDesc(dd));
            xd->m_replaying = false;
        },
                     dd, 0.0);
    }

    void HttpgdDev::event_hist_play()
    {
        later::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);

            if (xd->server.is_recording() && xd->m_needsave)
            {
                xd->history.push_current(dd);
                xd->m_needsave = false;
            }

            int index = xd->server.get_history_index();
            xd->m_replaying = true;
            xd->history.play(index, dd);
            xd->m_replaying = false;

            // notify size
            xd->hist_update_size();
        },
                     dd, 0.0);
    }
    void HttpgdDev::event_hist_clear()
    {
        later::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            xd->history.clear();
            xd->m_needsave = false;
            xd->server.set_history_size(0);
        },
                     dd, 0.0);
    }
} // namespace httpgd