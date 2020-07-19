
#include <Rcpp.h>
#include <string>
#include "HttpgdDev.h"
#include "RSync.h"

#include "lib/svglite_utils.h"

namespace httpgd
{
    HttpgdDev::HttpgdDev(const HttpgdServerConfig &t_config, const HttpgdDevStartParams &t_params)
        : devGeneric(t_params.width, t_params.height, t_params.pointsize),
          system_aliases(Rcpp::wrap(t_params.aliases["system"])),
          user_aliases(Rcpp::wrap(t_params.aliases["user"])),
          m_history(std::string(".httpgdPlots_").append(random_token(4)))
    {
        m_df_displaylist = true;

        m_svr_config = std::make_shared<HttpgdServerConfig>(t_config);
        m_data_store = std::make_shared<HttpgdDataStore>();
        // setup callbacks
        m_data_store->notify_hist_clear = [&](bool async) {
            if (async)
            {
                await_hist_clear();
            }
            else
            {
                hist_clear();
            }
        };
        m_data_store->notify_replay = [&](bool async, int index, double width, double height) {
            if (async)
            {
                await_render_page(index, width, height);
            }
            else
            {
                render_page(index, width, height);
            }
        };
        m_data_store->notify_hist_remove = [&](bool async, int index) {
            if (async)
            {
                await_hist_remove(index);
            }
            else
            {
                hist_remove(index);
            }
        };

        // setup http server
        m_svr_task = std::make_shared<http::HttpgdHttpTask>(m_svr_config, m_data_store);
        //m_svr_task = new http::HttpgdHttpTask(m_svr_config, m_data_store);
    }
    HttpgdDev::~HttpgdDev()
    {
        Rcpp::Rcout << "Httpgd Device destroyed! \n";
    }

    // DEVICE CALLBACKS

    void HttpgdDev::dev_activate(pDevDesc dd)
    {
    }
    void HttpgdDev::dev_deactivate(pDevDesc dd)
    {
    }

    void HttpgdDev::dev_mode(int mode, pDevDesc dd)
    {
        /*if (!server.replaying) {
            if (start_draw) {
                rsync::lock();
            } else {
                rsync::unlock();
            }
        }*/
    }

    void HttpgdDev::dev_close(pDevDesc dd)
    {
        Rcpp::Rcout << "Server closing... ";
        rsync::awaitLater();
        rsync::lock();
        rsync::unlock();

        hist_clear();
        shutdown_server();

        Rcpp::Rcout << "Closed.\n";
    }

    void HttpgdDev::dev_metricInfo(int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd)
    {
        if (c < 0)
        {
            c = -c;
        }

        std::pair<std::string, int> font = get_font_file(gc->fontfamily, gc->fontface, user_aliases);

        int error = glyph_metrics(c, font.first.c_str(), font.second, gc->ps * gc->cex, 1e4, ascent, descent, width);
        if (error != 0)
        {
            *ascent = 0;
            *descent = 0;
            *width = 0;
        }
        double mod = 72. / 1e4;
        *ascent *= mod;
        *descent *= mod;
        *width *= mod;
    }
    double HttpgdDev::dev_strWidth(const char *str, pGEcontext gc, pDevDesc dd)
    {
        std::pair<std::string, int> font = get_font_file(gc->fontfamily, gc->fontface, user_aliases);

        double width = 0.0;

        int error = string_width(str, font.first.c_str(), font.second, gc->ps * gc->cex, 1e4, 1, &width);

        if (error != 0)
        {
            width = 0.0;
        }

        return width * 72. / 1e4;
    }

    void HttpgdDev::dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd)
    {
        if (m_target == -1)
            return;
        m_data_store->page_clip(m_target, x0, x1, y0, y1);
    }
    void HttpgdDev::dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd)
    {
    }
    void HttpgdDev::resize_device_to_page(pDevDesc dd)
    {
        int t = (m_target == -1) ? m_target_open : m_target;

	    dd->left = 0.0;
	    dd->top = 0.0;
	    dd->right = m_data_store->page_get_width(t);
	    dd->bottom = m_data_store->page_get_height(t);
    }

    void HttpgdDev::dev_newPage(pGEcontext gc, pDevDesc dd)
    {
        const double width = dd->right;
        const double height = dd->bottom;
        const int fill = dd->startfill; 

        // Rcpp::Rcout << "[new_page] replaying="<<replaying<<"\n";
        if (!replaying)
        {
            if (m_target_open >= 0) // no previous pages
            {
                // Rcpp::Rcout << "    -> record open page in history\n";
                m_history.set_last(m_target_open, dd);
            }
            // Rcpp::Rcout << "    -> add new page to server\n";
            m_target = m_data_store->page_new(width, height);
            m_target_open = m_target;
        }
        else
        {
            // Rcpp::Rcout << "    -> rewrite target: " << m_target << "\n";
            // Rcpp::Rcout << "    -> clear page\n";
            if (m_target != -1)
                m_data_store->page_clear(m_target, true);
        }
        if (m_target != -1)
            m_data_store->page_fill(m_target, fill);
    }
    void HttpgdDev::dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Line>(gc, x1, y1, x2, y2));
    }
    void HttpgdDev::dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Text>(gc, x, y, str, rot, hadj,
                                       dc::TextInfo{
                                           fontname(gc->fontfamily, gc->fontface, system_aliases, user_aliases),
                                           gc->cex * gc->ps,
                                           is_bold(gc->fontface),
                                           is_italic(gc->fontface),
                                           dev_strWidth(str, gc, dd)}));
    }
    void HttpgdDev::dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Rect>(gc, x0, y0, x1, y1));
    }
    void HttpgdDev::dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Circle>(gc, x, y, r));
    }
    void HttpgdDev::dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
        std::vector<double> vx(x, x + n);
        std::vector<double> vy(y, y + n);
        put(std::make_shared<dc::Polygon>(gc, n, vx, vy));
    }
    void HttpgdDev::dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
        std::vector<double> vx(x, x + n);
        std::vector<double> vy(y, y + n);
        put(std::make_shared<dc::Polyline>(gc, n, vx, vy));
    }
    void HttpgdDev::dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd)
    {
        std::vector<int> vnper(nper, nper + npoly);
        int npoints = 0;
        for (int i = 0; i < npoly; i++)
        {
            npoints += vnper[i];
        }
        std::vector<double> vx(x, x + npoints);
        std::vector<double> vy(y, y + npoints);

        put(std::make_shared<dc::Path>(gc, vx, vy, npoly, vnper, winding));
    }
    void HttpgdDev::dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd)
    {
        std::vector<unsigned int> raster_(raster, raster + (w * h));
        put(std::make_shared<dc::Raster>(gc, raster_, w, h, x, y, width, height, rot, interpolate));
    }

    // OTHER

    void HttpgdDev::await_render_page(int target, double width, double height)
    {
        replaying_index = target;
        replaying_width = width;
        replaying_height = height;
        rsync::later([](void *dd) {
            auto xd = static_cast<HttpgdDev *>(dd);
            xd->replaying = true;
            xd->render_page(xd->replaying_index, xd->replaying_width, xd->replaying_height);
            xd->replaying = false;
        },
                     this, 0.0);
        rsync::awaitLater();
    }
    void HttpgdDev::await_hist_clear()
    {
        rsync::later([](void *dd) {
            auto xd = static_cast<HttpgdDev *>(dd);
            xd->replaying = true;
            xd->hist_clear();
            xd->replaying = false;
        },
                     this, 0.0);
        rsync::awaitLater();
    }
    void HttpgdDev::await_hist_remove(int target)
    {
        replaying_index = target;
        rsync::later([](void *dd) {
            auto xd = static_cast<HttpgdDev *>(dd);
            xd->replaying = true;
            xd->hist_remove(xd->replaying_index);
            xd->replaying = false;
        },
                     this, 0.0);
        rsync::awaitLater();
    }

    void HttpgdDev::put(std::shared_ptr<dc::DrawCall> dc)
    {
        if (m_target == -1)
            return;

        m_data_store->page_put(m_target, dc, replaying);
    }

    void HttpgdDev::render_page(int render_target, double width, double height)
    {
        pDevDesc dd = devGeneric::get_active_pDevDesc();

        // Rcpp::Rcout << "[render_page] render_target=" << render_target << "\n";

        if (render_target == -1)
            return;

        replaying = true;
        m_data_store->page_resize(render_target, width, height); // this also clears
        if (render_target == m_target_open)
        {
            m_target = render_target; //???
            // Rcpp::Rcout << "    -> open page. target_open="<< m_target << "\n";
            //dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            resize_device_to_page(dd);
            devGeneric::replay_current(dd); // replay active page
            //m_target = m_target_open;           // set target to open page for new draw calls
        }
        else
        {
            // Rcpp::Rcout << "    -> old page. target_open="<< m_target_open << "\n";
            m_history.set_current(m_target_open, dd);

            m_target = render_target;
            //dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            resize_device_to_page(dd);
            m_history.play(m_target, dd);
            m_target = -1;
            //dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
            resize_device_to_page(dd);
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
        pDevDesc dd = devGeneric::get_active_pDevDesc();

        // Rcpp::Rcout << "[hist_remove] target = " << target << "\n";
        replaying = true;
        m_history.remove(target);
        if (target == m_target_open && target > 0)
        {
            //Rcpp::Rcout << "   -> last removed replay new last\n";
            m_target = m_target_open - 1;
            resize_device_to_page(dd);
            m_history.play(m_target_open - 1, dd); // recreate state of the element before last element
        }
        m_target_open--;
        replaying = false;
    }

    void HttpgdDev::start_server()
    {
        m_svr_task->begin();
    }
    void HttpgdDev::shutdown_server()
    {
        m_svr_task->stop();
    }
    int HttpgdDev::server_await_port()
    {
        return m_svr_task->await_port();
    }
    std::string HttpgdDev::store_state_json(bool include_config)
    {
        if (include_config)
        {
            return m_data_store->api_state_json(); //todo &m_svr_config, m_svr_task.await_port());
        }
        else
        {
            return m_data_store->api_state_json();
        }
    }
    std::string HttpgdDev::store_svg(int index, double width, double height)
    {
        return m_data_store->api_svg(false, index, width, height);
    }
    bool HttpgdDev::store_remove(int index)
    {
        return m_data_store->api_remove(false, index);
    }
    bool HttpgdDev::store_clear()
    {
        return m_data_store->api_clear(false);
    }
    std::shared_ptr<HttpgdServerConfig> HttpgdDev::get_server_config()
    {
        return m_svr_config;
    }
    int HttpgdDev::store_get_upid()
    {
        return m_data_store->get_upid();
    }
    int HttpgdDev::store_get_page_count()
    {
        return m_data_store->page_count();
    }

    // Generate random alphanumeric string with R's built in RNG
    // https://cran.rstudio.com/doc/manuals/r-devel/R-exts.html#Random-numbers
    std::string HttpgdDev::random_token(int len)
    {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string s(len, 'a');
        GetRNGstate();
        for (int i = 0; i < len; i++)
        {
            s[i] = alphanum[(int)(unif_rand() * (sizeof(alphanum) / sizeof(*alphanum) - 1))];
        }
        PutRNGstate();
        return s;
    }
} // namespace httpgd