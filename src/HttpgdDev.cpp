

#include <cpp11/strings.hpp>
#include <cpp11/list.hpp>
#include <cpp11/as.hpp>
#include <string>
#include <random>
#include <cmath>
//#include "graphicsInternals.h"

#include "HttpgdDev.h"

#include "lib/svglite_utils.h"

namespace httpgd
{
    int DeviceTarget::get_index() const { return m_index; }
    void DeviceTarget::set_index(int t_index)
    {
        m_void = false;
        m_index = t_index;
    }
    int DeviceTarget::get_newest_index() const
    {
        return m_newest_index;
    }
    void DeviceTarget::set_newest_index(int t_index)
    {
        m_newest_index = t_index;
    }
    bool DeviceTarget::is_void() const
    {
        return m_void;
    }
    void DeviceTarget::set_void()
    {
        m_void = true;
        m_index = -1;
    }

    HttpgdDev::HttpgdDev(const HttpgdServerConfig &t_config, const HttpgdDevStartParams &t_params)
        : devGeneric(t_params.width, t_params.height, t_params.pointsize, t_params.bg),
          system_aliases(cpp11::as_cpp<cpp11::list>(t_params.aliases["system"])),
          user_aliases(cpp11::as_cpp<cpp11::list>(t_params.aliases["user"])),
          m_history()
    {
        m_df_displaylist = true;

        m_svr_config = std::make_shared<HttpgdServerConfig>(t_config);
        m_data_store = std::make_shared<HttpgdDataStore>();
        m_api_async_watcher = std::make_shared<HttpgdApiAsync>(this, m_svr_config, m_data_store);

        // setup http server
        m_server = m_svr_config->webserver ? std::make_shared<web::WebServer>(m_api_async_watcher) : nullptr;

        m_initialized = true;
    }
    HttpgdDev::~HttpgdDev()
    {
        //Rcpp::Rcout << "Httpgd Device destructed.\n";
    }

    // DEVICE CALLBACKS

    void HttpgdDev::dev_activate(pDevDesc dd)
    {
        if (!m_initialized) return;
        //Rcpp::Rcout << "ACTIVATE 1\n";
        m_data_store->set_device_active(true);
        if (m_server && m_server_running) {
            HttpgdState state = m_data_store->state();
            state.active = true; // in case it has changed
            m_server->broadcast_state(state);
        }
    }
    void HttpgdDev::dev_deactivate(pDevDesc dd)
    {
        if (!m_initialized) return;
        //Rcpp::Rcout << "DEACTIVATE 0\n";
        m_data_store->set_device_active(false);
        if (m_server && m_server_running) {
            HttpgdState state = m_data_store->state();
            state.active = false; // in case it has changed
            m_server->broadcast_state(state);
        }
    }

    void HttpgdDev::dev_mode(int mode, pDevDesc dd)
    {
        //Rcpp::Rcout << "MODE "<<mode<<"\n";
        if (m_target.is_void() || mode == 1)
            return;
            
        if (m_server && m_server_running)
            m_server->broadcast_state_current();
    }

    void HttpgdDev::dev_close(pDevDesc dd)
    {
        m_initialized = false;

        if (m_server && !m_svr_config->silent)
            Rprintf("Server closing... ");

        // notify watcher
        m_api_async_watcher->rdevice_destructing();

        // stop accepting draw calls
        m_target.set_void();
        m_target.set_newest_index(-1);

        // shutdown http server
        server_stop();

        // cleanup r session data
        m_history.clear();

        if (m_server && !m_svr_config->silent)
            Rprintf("Closed.\n");
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
        if (m_target.is_void())
            return;
        m_data_store->clip(m_target.get_index(), x0, x1, y0, y1);
    }
    void HttpgdDev::dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd)
    {
    }

    /*inline httpgd::HttpgdDataStorePageSize find_minsize() {
        Rcpp::Function rpar("par");
        Rcpp::List res = rpar();
        Rcpp::NumericVector mai = res["mai"];
        double minw = (mai[1]+mai[3]) * 72 + 1;
        double minh = (mai[0]+mai[2]) * 72 + 1;
        return {minw, minh};
    }*/
    
    /*inline httpgd::HttpgdDataStorePageSize find_minsize(const pDevDesc &dd) {
        auto mai = getGPar(desc2GEDesc(dd)).mai;
        double minw = (mai[1]+mai[3]) * 72 + 1;
        double minh = (mai[0]+mai[2]) * 72 + 1;
        return {minw, minh};
    }*/

    void HttpgdDev::resize_device_to_page(pDevDesc dd)
    {
        int index = (m_target.is_void()) ? m_target.get_newest_index() : m_target.get_index();

        auto size = m_data_store->size(index);
        //auto minsize = find_minsize(dd);

        dd->left = 0.0;
        dd->top = 0.0;
        //dd->right = std::max(size.width, minsize.width);
        //dd->bottom = std::max(size.height, minsize.height);
        dd->right = size.width;
        dd->bottom = size.height;
    }

    void HttpgdDev::dev_newPage(pGEcontext gc, pDevDesc dd)
    {
        const double width = dd->right;
        const double height = dd->bottom;
        const int fill = (R_ALPHA(gc->fill) == 0) ? dd->startfill : gc->fill;

        // Rcpp::Rcout << "[new_page] replaying="<<replaying<<"\n";
        if (!replaying)
        {
            if (m_target.get_newest_index() >= 0) // no previous pages
            {
                // Rcpp::Rcout << "    -> record open page in history\n";
                m_history.put_last(m_target.get_newest_index(), dd);
            }
            // Rcpp::Rcout << "    -> add new page to server\n";
            m_target.set_index(m_data_store->append(width, height));
            m_target.set_newest_index(m_target.get_index());
        }
        else
        {
            // Rcpp::Rcout << "    -> rewrite target: " << m_target << "\n";
            // Rcpp::Rcout << "    -> clear page\n";
            if (!m_target.is_void())
                m_data_store->clear(m_target.get_index(), true);
        }
        if (!m_target.is_void())
            m_data_store->fill(m_target.get_index(), fill);
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

    void HttpgdDev::put(std::shared_ptr<dc::DrawCall> dc)
    {
        if (m_target.is_void())
            return;

        m_data_store->add_dc(m_target.get_index(), dc, replaying);
    }

    void HttpgdDev::api_render(int index, double width, double height)
    {
        if (index == -1) index = m_target.get_newest_index();
        
        pDevDesc dd = devGeneric::get_active_pDevDesc();

        // Rcpp::Rcout << "[render_page] index=" << index << "\n";

        replaying = true;
        m_data_store->resize(index, width, height); // this also clears
        if (index == m_target.get_newest_index())
        {
            m_target.set_index(index); //???
            // Rcpp::Rcout << "    -> open page. target_index=" << m_target.get_index() << "\n";
            resize_device_to_page(dd);
            devGeneric::replay_current(dd); // replay active page
        }
        else
        {
            // Rcpp::Rcout << "    -> old page. target_newest_index="<< m_target.get_newest_index() << "\n";
            m_history.put_current(m_target.get_newest_index(), dd);

            m_target.set_index(index);
            resize_device_to_page(dd);
            m_history.play(m_target.get_index(), dd);
            m_target.set_void();
            resize_device_to_page(dd);
            m_history.play(m_target.get_newest_index(), dd); // recreate previous state
            m_target.set_index(m_target.get_newest_index());    // set target to open page for new draw calls
        }
        replaying = false;
    }

    bool HttpgdDev::api_clear()
    {
        // clear store
        bool r = m_data_store->remove_all();

        // clear history
        m_history.clear();
        m_target.set_void();
        m_target.set_newest_index(-1);

        return r;
    }

    bool HttpgdDev::api_remove(int index)
    {
        if (index == -1) index = m_target.get_newest_index();

        // remove from store
        bool r = m_data_store->remove(index, false);

        // remove from history

        pDevDesc dd = devGeneric::get_active_pDevDesc();

        // Rcpp::Rcout << "[hist_remove] target = " << target << "\n";
        replaying = true;
        m_history.remove(index);
        if (index == m_target.get_newest_index() && index > 0)
        {
            //Rcpp::Rcout << "   -> last removed replay new last\n";
            m_target.set_index(m_target.get_newest_index() - 1);
            resize_device_to_page(dd);
            m_history.play(m_target.get_newest_index() - 1, dd); // recreate state of the element before last element
        }
        m_target.set_newest_index(m_target.get_newest_index() - 1);
        replaying = false;
        
        return r;
    }

    void HttpgdDev::api_svg(std::ostream &os, int index, double width, double height)
    {
        // Rcpp::Rcout << "DIFF \n";
        if (m_data_store->diff(index, width, height))
        {
            // Rcpp::Rcout << "RENDER \n";
            api_render(index, width, height);
        }
        // Rcpp::Rcout << "SVG \n";
        m_data_store->svg(os, index);
    }

    bool HttpgdDev::server_start()
    {
        if (m_server && !m_server_running) {
            m_server_running = m_server->start();
            return m_server_running;
        }
        return true;
    }
    void HttpgdDev::server_stop()
    {
        if (m_server && m_server_running)
            m_server->stop();
        
        m_server_running = false;
    }
    unsigned short HttpgdDev::server_port() const
    {
        return m_server ? m_server->port() : 0;
    }

    std::shared_ptr<HttpgdServerConfig> HttpgdDev::api_server_config()
    {
        return m_svr_config;
    }

    /*int HttpgdDev::api_upid()
    {
        return m_data_store->upid();
    }
    bool HttpgdDev::api_active()
    {
        return m_data_store->device_active();
    }
    int HttpgdDev::api_page_count()
    {
        return m_data_store->count();
    }*/
    HttpgdState HttpgdDev::api_state()
    {
        return m_data_store->state();
    }

    // Security vulnerability: Seed can not be chosen if R's RNG is used
    // Generate random alphanumeric string with R's built in RNG
    // https://cran.rstudio.com/doc/manuals/r-devel/R-exts.html#Random-numbers
    /*std::string HttpgdDev::random_token(int len)
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
    }*/

    std::string HttpgdDev::random_token(int len)
    {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        static auto rseed = static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        static std::mt19937 generator(rseed);
        static std::uniform_int_distribution<int> distribution{0, static_cast<int>((sizeof(alphanum) / sizeof(alphanum[0])) - 2)};

        std::string rand_str(len, '\0');
        for (int i = 0; i < len; ++i)
        {
            rand_str[i] = alphanum[distribution(generator)];
        }

        return rand_str;
    }

} // namespace httpgd