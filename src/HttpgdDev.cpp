
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

    HttpgdDev::HttpgdDev(pDevDesc t_dd, const HttpgdDevStartParams &params)
        : dd(t_dd),
          server(params.host, params.port, params.width, params.height, params.recording, params.cors, params.use_token, params.token),
          history(HISTORY_MAGIC, 4, std::string(".httpgdPlots_").append(std::to_string(params.port))),
          font(params.aliases)
    {
        // setup callbacks
        server.notify_hist_clear = [&]() { event_hist_clear(); };
        server.notify_replay = [&]() {
            if (server.replaying) // this should always be true (can't assert)
            { 
                later::later([](void *ddp) {
                    Rcpp::Rcout << "Replay\n";
                    auto dd = static_cast<pDevDesc>(ddp);
                    auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);

                    if (xd->server.needsave && xd->server.history_recording)
                    {
                        xd->history.push_current(dd);
                        xd->server.needsave = false;
                    }

                    // resize
                    dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
                    
                    if (xd->server.last_page() && xd->server.needsave) {
                        GEplayDisplayList(desc2GEDesc(dd)); // replay active page
                    } else {
                        xd->history.play(xd->server.history_index, dd); // replay from history
                    }
                    xd->server.replaying = false;
                },
                             dd, 0.0);
            }
        };

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
        if (server.history_recording && server.needsave)
        {
            history.push_last(dd);
        }
        server.needsave = !server.replaying;
        if (!server.replaying)
        {
            hist_update_size();
        }
    }
    void HttpgdDev::hist_update_size()
    {
        server.history_size = history.size() + (server.needsave ? 1 : 0);
    }

    void HttpgdDev::event_resized()
    {
        //m_replaying = true; // replaying should already be true at this point
        later::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            GEplayDisplayList(desc2GEDesc(dd));
            xd->server.replaying = false;
        },
                     dd, 0.0);
    }

    void HttpgdDev::event_hist_play()
    {
        later::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);

            if (xd->server.history_recording && xd->server.needsave)
            {
                xd->history.push_current(dd);
                xd->server.needsave = false;
            }

            int index = xd->server.history_index;
            xd->server.replaying = true;
            xd->history.play(index, dd);
            xd->server.replaying = false;

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
            xd->server.needsave = false;
            xd->server.history_size = 0;
            xd->server.history_index = -1;

            xd->server.replaying = false;
        },
                     dd, 0.0);
    }
} // namespace httpgd