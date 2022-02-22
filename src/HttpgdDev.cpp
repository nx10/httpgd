
#include "HttpgdDev.h"
#include "DebugPrint.h"

#include <cmath>
#include <cpp11/as.hpp>
#include <cpp11/doubles.hpp>
#include <cpp11/function.hpp>
#include <cpp11/list.hpp>
#include <cpp11/strings.hpp>
#include <svglite_utils.h>
#include <string>

namespace httpgd
{
    int DeviceTarget::get_index() const
    {
        return m_index;
    }
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
          m_history(),
          m_fix_strwidth(t_params.fix_strwidth)
    {
        m_df_displaylist = true;

        m_svr_config = std::make_shared<HttpgdServerConfig>(t_config);
        m_data_store = std::make_shared<HttpgdDataStore>();
        m_data_store->extra_css(t_params.extra_css);
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
        if (!m_initialized)
            return;
        //Rcpp::Rcout << "ACTIVATE 1\n";
        m_data_store->set_device_active(true);
        if (m_server && m_server_running)
        {
            m_server->broadcast_state_current();
        }
    }
    void HttpgdDev::dev_deactivate(pDevDesc dd)
    {
        if (!m_initialized)
            return;
        //Rcpp::Rcout << "DEACTIVATE 0\n";
        m_data_store->set_device_active(false);
        if (m_server && m_server_running)
        {
            m_server->broadcast_state_current();
        }
    }

    void HttpgdDev::dev_mode(int mode, pDevDesc dd)
    {
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

        FontSettings font = get_font_file(gc->fontfamily, gc->fontface, user_aliases);

        int error = glyph_metrics(c, font.file, font.index, gc->ps * gc->cex, 1e4, ascent, descent, width);
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
        FontSettings font = get_font_file(gc->fontfamily, gc->fontface, user_aliases);

        double width = 0.0;

        int error = string_width(str, font.file, font.index, gc->ps * gc->cex, 1e4, 1, &width);

        if (error != 0)
        {
            width = 0.0;
        }

        return width * 72. / 1e4;
    }

    void HttpgdDev::dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd)
    {
        if (m_target.is_void())
        {
            return;
        }
        m_data_store->clip(m_target.get_index(), normalize_rect(x0, y0, x1, y1));
    }
    void HttpgdDev::dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd)
    {
    }

    /**
     * "Figure margins too large" protection.
     * Including the graphics headers and reading the values directly
     * is about 40 times faster, but is probably not allowed by CRAN.
     */
    inline gvertex<double> find_minsize()
    {
        const auto par = cpp11::package("graphics")["par"];
        const auto mai = cpp11::as_cpp<cpp11::doubles>(cpp11::as_cpp<cpp11::list>(par())["mai"]);
        const double minw = (mai[1] + mai[3]) * 72 + 1;
        const double minh = (mai[0] + mai[2]) * 72 + 1;
        return {minw, minh};
    }

    void HttpgdDev::resize_device_to_page(pDevDesc dd)
    {
        int index = (m_target.is_void()) ? m_target.get_newest_index() : m_target.get_index();

        auto size = m_data_store->size(index);
        auto minsize = find_minsize();

        dd->left = 0.0;
        dd->top = 0.0;
        dd->right = std::max(size.x, minsize.x);
        dd->bottom = std::max(size.y, minsize.y);
    }

    void HttpgdDev::dev_newPage(pGEcontext gc, pDevDesc dd)
    {
        const double width = dd->right;
        const double height = dd->bottom;
        const int fill = (R_ALPHA(gc->fill) == 0) ? dd->startfill : gc->fill;

        debug_print("[new_page] replaying=%i\n", replaying);
        if (!replaying)
        {
            if (m_target.get_newest_index() >= 0) // no previous pages
            {
                debug_print("    -> record open page in history\n");
                m_history.put_last(m_target.get_newest_index(), dd);
            }
            debug_print("    -> add new page to server\n");
            m_target.set_index(m_data_store->append({width, height}));
            m_target.set_newest_index(m_target.get_index());
        }
        else
        {
            debug_print("    -> rewrite target: %i\n", m_target.get_index());
            debug_print("    -> clear page\n");
            if (!m_target.is_void())
                m_data_store->clear(m_target.get_index(), true);
        }
        if (!m_target.is_void())
            m_data_store->fill(m_target.get_index(), fill);
    }

    inline dc::LineInfo gc_lineinfo(pGEcontext gc)
    {
        return {
            gc->col,
            gc->lwd,
            gc->lty,
            static_cast<dc::LineInfo::GC_lineend>(gc->lend),
            static_cast<dc::LineInfo::GC_linejoin>(gc->ljoin),
            gc->lmitre};
    }
    inline color_t gc_fill(pGEcontext gc)
    {
        return gc->fill;
    }

    void HttpgdDev::dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Line>(gc_lineinfo(gc), gvertex<double>{x1, y1}, gvertex<double>{x2, y2}));
    }
    void HttpgdDev::dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd)
    {
        FontSettings font_info = get_font_file(gc->fontfamily, gc->fontface, user_aliases);

        int weight = get_font_weight(font_info.file, font_info.index);

        std::string feature = "";
        for (int i = 0; i < font_info.n_features; ++i)
        {
            feature += "'";
            feature += font_info.features[i].feature[0];
            feature += font_info.features[i].feature[1];
            feature += font_info.features[i].feature[2];
            feature += font_info.features[i].feature[3];
            feature += "' ";
            feature += font_info.features[i].setting;
            feature += (i == font_info.n_features - 1 ? ";" : ",");
        }

        put(std::make_shared<dc::Text>(gc->col, gvertex<double>{x, y}, str, rot, hadj,
                                       dc::TextInfo{
                                           weight,
                                           feature,
                                           fontname(gc->fontfamily, gc->fontface, system_aliases, user_aliases, font_info),
                                           gc->cex * gc->ps,
                                           is_italic(gc->fontface),
                                           m_fix_strwidth ? dev_strWidth(str, gc, dd) : -1.0}));
    }
    void HttpgdDev::dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Rect>(gc_lineinfo(gc), gc_fill(gc), normalize_rect(x0, y0, x1, y1)));
    }
    void HttpgdDev::dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd)
    {
        put(std::make_shared<dc::Circle>(gc_lineinfo(gc), gc_fill(gc), gvertex<double>{x, y}, r));
    }
    void HttpgdDev::dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
        std::vector<gvertex<double>> points(n);
        for (int i = 0; i < n; ++i)
        {
            points[i] = {x[i], y[i]};
        }
        put(std::make_shared<dc::Polygon>(gc_lineinfo(gc), gc_fill(gc), std::move(points)));
    }
    void HttpgdDev::dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
        std::vector<gvertex<double>> points(n);
        for (int i = 0; i < n; ++i)
        {
            points[i] = {x[i], y[i]};
        }
        put(std::make_shared<dc::Polyline>(gc_lineinfo(gc), std::move(points)));
    }
    void HttpgdDev::dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd)
    {
        std::vector<int> vnper(nper, nper + npoly);
        int npoints = 0;
        for (const auto &val : vnper)
        {
            npoints += val;
        }
        std::vector<gvertex<double>> points(npoints);
        for (int i = 0; i < npoints; ++i)
        {
            points[i] = {x[i], y[i]};
        }

        put(std::make_shared<dc::Path>(gc_lineinfo(gc), gc_fill(gc), std::move(points), std::move(vnper), winding));
    }
    void HttpgdDev::dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd)
    {
        const double abs_height = std::fabs(height);
        const double abs_width = std::fabs(width);

        std::vector<unsigned int> vraster(raster, raster + (w * h));
        put(std::make_shared<dc::Raster>(std::move(vraster), gvertex<int>{w, h}, grect<double>{x, y - abs_height, abs_width, abs_height}, rot, interpolate));
    }

    // OTHER

    void HttpgdDev::put(std::shared_ptr<dc::DrawCall> dc)
    {
        if (m_target.is_void())
            return;

        m_data_store->add_dc(m_target.get_index(), dc, replaying);
    }

    void HttpgdDev::api_prerender(int index, double width, double height)
    {
        if (index == -1)
            index = m_target.get_newest_index();

        pDevDesc dd = devGeneric::get_active_pDevDesc();

        debug_print("[render_page] index=%i\n", index);

        replaying = true;
        m_data_store->resize(index, {width, height}); // this also clears
        if (index == m_target.get_newest_index())
        {
            m_target.set_index(index);
            debug_print("    -> open page. target_index=%i\n", m_target.get_index());
            resize_device_to_page(dd);
            PlotHistory::replay_current(dd); // replay active page
        }
        else
        {
            debug_print("    -> old page. target_newest_index=%i\n", m_target.get_newest_index());
            m_history.put_current(m_target.get_newest_index(), dd);

            m_target.set_index(index);
            resize_device_to_page(dd);
            m_history.play(m_target.get_index(), dd);
            m_target.set_void();
            resize_device_to_page(dd);
            m_history.play(m_target.get_newest_index(), dd); // recreate previous state
            m_target.set_index(m_target.get_newest_index()); // set target to open page for new draw calls
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

        if (m_server && m_server_running)
            m_server->broadcast_state_current();

        return r;
    }

    bool HttpgdDev::api_remove(int index)
    {
        if (index == -1)
            index = m_target.get_newest_index();

        // remove from store
        bool r = m_data_store->remove(index, false);

        // remove from history

        pDevDesc dd = devGeneric::get_active_pDevDesc();

        debug_print("[hist_remove] index = %i\n", index);
        replaying = true;
        m_history.remove(index);
        if (index == m_target.get_newest_index() && index > 0)
        {
            debug_print("   -> last removed replay new last\n");
            m_target.set_index(m_target.get_newest_index() - 1);
            resize_device_to_page(dd);
            m_history.play(m_target.get_newest_index() - 1, dd); // recreate state of the element before last element
        }
        m_target.set_newest_index(m_target.get_newest_index() - 1);
        replaying = false;

        if (m_server && m_server_running)
            m_server->broadcast_state_current();

        return r;
    }
    
    bool HttpgdDev::api_render(int index, double width, double height, dc::RenderingTarget *t_renderer, double t_scale) 
    {
        debug_print("DIFF \n");
        if (m_data_store->diff(index, {width, height}))
        {
            debug_print("RENDER \n");
            api_prerender(index, width, height);
        }
        debug_print("SVG \n");
        return m_data_store->render(index, t_renderer, t_scale);
    }

    boost::optional<int> HttpgdDev::api_index(int32_t id)
    {
        return m_data_store->find_index(id);
    }

    bool HttpgdDev::server_start()
    {
        if (m_server && !m_server_running)
        {
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

    HttpgdState HttpgdDev::api_state()
    {
        return m_data_store->state();
    }

    HttpgdQueryResults HttpgdDev::api_query_all()
    {
        return m_data_store->query_all();
    }
    HttpgdQueryResults HttpgdDev::api_query_index(int index)
    {
        return m_data_store->query_index(index);
    }
    HttpgdQueryResults HttpgdDev::api_query_range(int offset, int limit)
    {
        return m_data_store->query_range(offset, limit);
    }

} // namespace httpgd