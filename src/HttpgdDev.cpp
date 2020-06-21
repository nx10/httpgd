
#include <Rcpp.h>
#include <string>
#include "HttpgdDev.h"
#include "RSync.h"

namespace httpgd
{

    HttpgdDev::HttpgdDev(pDevDesc t_dd, const HttpgdServerConfig &t_config, const HttpgdDevStartParams &t_params)
        : dd(t_dd),
          font(t_params.aliases),
          replaying(false),  
          replaying_index(0),
          replaying_width(0),
          replaying_height(0),
          m_history(std::string(".httpgdPlots_").append(random_token(4))),
          m_data_store(),
          m_svr_config(t_config),
          m_svr_task(&m_data_store, &m_svr_config),
          m_target(-1), m_target_open(-1)
    {
        // setup callbacks
        m_data_store.notify_hist_clear = [&](bool async) {
            if (async)
            {
                await_hist_clear();
            }
            else
            {
                hist_clear();
            }
        };
        m_data_store.notify_replay = [&](bool async, int index, double width, double height) {
            if (async)
            {
                await_render_page(index, width, height);
            }
            else
            {
                render_page(index, width, height);
            }
        };
        m_data_store.notify_hist_remove = [&](bool async, int index) {
            if (async)
            {
                await_hist_remove(index);
            }
            else
            {
                hist_remove(index);
            }
        };
    }

    void HttpgdDev::await_render_page(int target, double width, double height)
    {
        replaying_index = target;
        replaying_width = width;
        replaying_height = height;
        rsync::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            xd->replaying = true;
            xd->render_page(xd->replaying_index, xd->replaying_width, xd->replaying_height);
            xd->replaying = false;
        },
                     dd, 0.0);
        rsync::awaitLater();
    }
    void HttpgdDev::await_hist_clear()
    {
        rsync::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            xd->replaying = true;
            xd->hist_clear();
            xd->replaying = false;
        },
                     dd, 0.0);
        rsync::awaitLater();
    }
    void HttpgdDev::await_hist_remove(int target)
    {
        replaying_index = target;
        rsync::later([](void *ddp) {
            auto dd = static_cast<pDevDesc>(ddp);
            auto xd = static_cast<HttpgdDev *>(dd->deviceSpecific);
            xd->replaying = true;
            xd->hist_remove(xd->replaying_index);
            xd->replaying = false;
        },
                     dd, 0.0);
        rsync::awaitLater();
    }

    HttpgdDev::~HttpgdDev() = default;

    void HttpgdDev::put(std::shared_ptr<dc::DrawCall> dc)
    {
        if (m_target == -1)
            return;

        m_data_store.page_put(m_target, dc, replaying);
    }

    void HttpgdDev::new_page(double width, double height, int fill)
    {
        // Rcpp::Rcout << "[new_page] replaying="<<replaying<<"\n";
        if (!replaying)
        {
            if (m_target_open >= 0) // no previous pages
            {
                // Rcpp::Rcout << "    -> record open page in history\n";
                m_history.set_last(m_target_open, dd);
            }
            // Rcpp::Rcout << "    -> add new page to server\n";
            m_target = m_data_store.page_new(width, height);
            m_target_open = m_target;
        }
        else
        {
            // Rcpp::Rcout << "    -> rewrite target: " << m_target << "\n";
            // Rcpp::Rcout << "    -> clear page\n";
            if (m_target != -1)
                m_data_store.page_clear(m_target, true);
        }
        if (m_target != -1)
            m_data_store.page_fill(m_target, fill);
    }
    void HttpgdDev::page_size(double *width, double *height)
    {
        int t = (m_target == -1) ? m_target_open : m_target;

        *width = m_data_store.page_get_width(t);
        *height = m_data_store.page_get_height(t);
    }
    void HttpgdDev::clip_page(double x0, double x1, double y0, double y1)
    {
        if (m_target == -1)
            return;
        m_data_store.page_clip(m_target, x0, x1, y0, y1);
    }
    void HttpgdDev::mode(bool start_draw)
    {
        /*if (!server.replaying) {
            if (start_draw) {
                rsync::lock();
            } else {
                rsync::unlock();
            }
        }*/
    }

    void HttpgdDev::render_page(int render_target, double width, double height)
    {
        // Rcpp::Rcout << "[render_page] render_target=" << render_target << "\n";

        if (render_target == -1)
            return;

        replaying = true;
        m_data_store.page_resize(render_target, width, height); // this also clears
        if (render_target == m_target_open)
        {
            m_target = render_target; //???
            // Rcpp::Rcout << "    -> open page. target_open="<< m_target << "\n";
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            GEplayDisplayList(desc2GEDesc(dd)); // replay active page
            //m_target = m_target_open;           // set target to open page for new draw calls
        }
        else
        {
            // Rcpp::Rcout << "    -> old page. target_open="<< m_target_open << "\n";
            m_history.set_current(m_target_open, dd);

            m_target = render_target;
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            m_history.play(m_target, dd);
            m_target = -1;
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            m_history.play(m_target_open, dd); // recreate previous state
            m_target = m_target_open;          // set target to open page for new draw calls
        }
        replaying = false;
    }

    void HttpgdDev::hist_clear()
    {
        m_history.clear();
        m_target = -1;
        m_target_open = -1;
    }

    void HttpgdDev::hist_remove(int target)
    {
        //Rcpp::Rcout << "[hist_remove] target = " << target << "\n";
        replaying = true;
        m_history.remove(target);
        if (target == m_target_open && target > 0)
        {
            //Rcpp::Rcout << "   -> last removed replay new last\n";
            m_target = m_target_open - 1;
            dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            m_history.play(m_target_open - 1, dd); // recreate state of the element before last element
        }
        m_target_open--;
        replaying = false;
    }

    void HttpgdDev::start_server()
    {
        m_svr_task.begin();
    }
    void HttpgdDev::shutdown_server()
    {
        m_svr_task.stop();
    }
    int HttpgdDev::server_await_port()
    {
        return m_svr_task.await_port();
    }
    std::string HttpgdDev::store_state_json(bool include_config)
    {
        if (include_config)
        {
            return m_data_store.api_state_json(&m_svr_config, m_svr_task.await_port());
        }
        else
        {
            return m_data_store.api_state_json();
        }
    }
    std::string HttpgdDev::store_svg(int index, double width, double height)
    {
        return m_data_store.api_svg(false, index, width, height);
    }
    bool HttpgdDev::store_remove(int index)
    {
        return m_data_store.api_remove(false, index);
    }
    bool HttpgdDev::store_clear()
    {
        return m_data_store.api_clear(false);
    }
    const HttpgdServerConfig *HttpgdDev::get_server_config()
    {
        return &m_svr_config;
    }
    int HttpgdDev::store_get_upid()
    {
        return m_data_store.get_upid();
    }
    int HttpgdDev::store_get_page_count()
    {
        return m_data_store.page_count();
    }

} // namespace httpgd