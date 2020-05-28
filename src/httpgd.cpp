// [[Rcpp::plugins("cpp11")]]


#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include <vector>
#include <string>

#include "HttpgdDev.h"
#include "DrawData.h"
#include "HttpgdServer.h"

#include "fixsuspinter.h"

#define LOGDRAW 0

namespace httpgd
{

    inline HttpgdDev *getDev(pDevDesc dd)
    {
        return static_cast<HttpgdDev *>(dd->deviceSpecific);
    }

    // --------------------------------------

    /**
 * R Callback: Get singe char font metrics.
 */
    void httpgd_metric_info(int c, const pGEcontext gc, double *ascent,
                            double *descent, double *width, pDevDesc dd)
    {
        HttpgdDev *dev = getDev(dd);

        dev->font.analyze(char_r_unicode(c), gc);

        *ascent = dev->font.get_ascent();
        *descent = dev->font.get_descent();
        *width = dev->font.get_width();

#if LOGDRAW == 1
        Rprintf("METRIC_INFO c=%i ascent=%f descent=%f width=%f\n", c, ascent, descent, width);
#endif
    }

    /**
 * R Callback: Get String width.
 */
    double httpgd_strwidth(const char *str, const pGEcontext gc, pDevDesc dd)
    {

#if LOGDRAW == 1
        Rprintf("STRWIDTH str=\"%s\"\n", str);
#endif

        HttpgdDev *dev = getDev(dd);

        dev->font.analyze(std::string(str), gc);

        return dev->font.get_width();
    }

    /**
 * R Callback: Clip draw area.
 */
    void httpgd_clip(double x0, double x1, double y0, double y1, pDevDesc dd)
    {
        getDev(dd)->server.page_clip(x0, x1, y0, y1);
#if LOGDRAW == 1
        Rprintf("CLIP x0=%f x1=%f y0=%f y1=%f\n", x0, x1, y0, y1);
#endif
    }

    /**
 * R Callback: Start new page.
 */
    void httpgd_new_page(const pGEcontext gc, pDevDesc dd)
    {
        HttpgdDev *dev = getDev(dd);

        dev->hist_new_page();

        dev->server.page_clear();
        dev->server.page_fill(dd->startfill); // todo should this be gc->fill ?


#if LOGDRAW == 1
        Rcpp::Rcout << "NEW_PAGE \n";
#endif
    }

    /**
 * R Callback: Close graphics device.
 */
    void httpgd_close(pDevDesc dd)
    {
        Rcpp::Rcout << "Server closing... ";

        HttpgdDev *dev = getDev(dd);
        dev->history.clear();
        dev->server.stop();
        free(dev);

        Rcpp::Rcout << "Closed.\n";

#if LOGDRAW == 1
        Rcpp::Rcout << "CLOSE \n";
#endif
    }

    // -------------------------------------------
    // Draw Objects.
    // -------------------------------------------

    /**
 * R Callback: Draw line.
 */
    void httpgd_line(double x1, double y1, double x2, double y2,
                     const pGEcontext gc, pDevDesc dd)
    {
        getDev(dd)->server.page_put(new dc::Line(gc, x1, y1, x2, y2));

#if LOGDRAW == 1
        Rprintf("LINE x1=%f y1=%f x2=%f y2=%f\n", x1, y1, x2, y2);
#endif
    }

    /**
 * R Callback: Draw polyline.
 */
    void httpgd_polyline(int n, double *x, double *y, const pGEcontext gc,
                         pDevDesc dd)
    {

        std::vector<double> vx(x, x + n);
        std::vector<double> vy(y, y + n);

        getDev(dd)->server.page_put(new dc::Polyline(gc, n, vx, vy));

#if LOGDRAW == 1
        Rcpp::Rcout << "POLYLINE \n";
#endif
    }

    /**
 * R Callback: Draw polygon.
 */
    void httpgd_polygon(int n, double *x, double *y, const pGEcontext gc,
                        pDevDesc dd)
    {

        std::vector<double> vx(x, x + n);
        std::vector<double> vy(y, y + n);

        getDev(dd)->server.page_put(new dc::Polygon(gc, n, vx, vy));

#if LOGDRAW == 1
        Rcpp::Rcout << "POLYGON \n";
#endif
    }

    /**
 * R Callback: Draw path.
 */
    void httpgd_path(double *x, double *y,
                     int npoly, int *nper,
                     Rboolean winding,
                     const pGEcontext gc, pDevDesc dd)
    {
        std::vector<int> vnper(nper, nper + npoly);
        int npoints = 0;
        for (int i = 0; i < npoly; i++)
        {
            npoints += vnper[i];
        }
        std::vector<double> vx(x, x + npoints);
        std::vector<double> vy(y, y + npoints);

        getDev(dd)->server.page_put(new dc::Path(gc, vx, vy, npoly, vnper, winding));

#if LOGDRAW == 1
        Rcpp::Rcout << "PATH \n";
#endif
    }

    /**
 * R Callback: Draw rectangle.
 */
    void httpgd_rect(double x0, double y0, double x1, double y1,
                     const pGEcontext gc, pDevDesc dd)
    {
        getDev(dd)->server.page_put(new dc::Rect(gc, x0, y0, x1, y1));

#if LOGDRAW == 1
        Rprintf("RECT x0=%f y0=%f x1=%f y1=%f\n", x0, y0, x1, y1);
#endif
    }

    /**
 * R Callback: Draw circle.
 */
    void httpgd_circle(double x, double y, double r, const pGEcontext gc,
                       pDevDesc dd)
    {
        getDev(dd)->server.page_put(new dc::Circle(gc, x, y, r));

#if LOGDRAW == 1
        Rprintf("CIRCLE x=%f y=%f r=%f\n", x, y, r);
#endif
    }

    /**
 * R Callback: Draw text.
 */
    void httpgd_text(double x, double y, const char *str, double rot,
                     double hadj, const pGEcontext gc, pDevDesc dd)
    {

        HttpgdDev *dev = getDev(dd);

        dev->font.analyze(std::string(str), gc);
        dev->server.page_put(new dc::Text(gc, x, y, str, rot, hadj,
                                      dc::TextInfo{
                                          dev->font.get_font_family(),
                                          dev->font.get_fontsize(),
                                          dev->font.is_bold(),
                                          dev->font.is_italic(),
                                          dev->font.get_width()}));

#if LOGDRAW == 1
        Rprintf("TEXT x=%f y=%f str=\"%s\" rot=%f hadj=%f\n", x, y, str, rot, hadj);
#endif
    }

    /**
 * R Callback: Get size of drawing.
 */
    void httpgd_size(double *left, double *right, double *bottom, double *top,
                     pDevDesc dd)
    {
        HttpgdDev *dev = getDev(dd);

        *left = 0.0;
        *right = dev->server.page_get_width();
        *bottom = dev->server.page_get_height();
        *top = 0.0;

#if LOGDRAW == 1
        Rprintf("SIZE left=%f right=%f bottom=%f top=%f\n", *left, *right, *bottom, *top);
#endif
    }

    /**
 * R Callback: Draw raster graphic.
 */
    void httpgd_raster(unsigned int *raster, int w, int h,
                       double x, double y,
                       double width, double height,
                       double rot,
                       Rboolean interpolate,
                       const pGEcontext gc, pDevDesc dd)
    {

        std::vector<unsigned int> raster_(raster, raster + (w * h));

        getDev(dd)->server.page_put(new dc::Raster(gc, raster_, w, h, x, y, width, height, rot, interpolate));

#if LOGDRAW == 1
        Rcpp::Rcout << "RASTER \n";
#endif
    }

    /**
 * R Callback: start draw = 1, stop draw = 0
 */
    static void httpgd_mode(int mode, pDevDesc dd)
    {

#if LOGDRAW == 1
        Rprintf("MODE mode=%i\n", mode);
#endif
    }

    // --------------------------------------

    pDevDesc httpgd_driver_new(const std::string &host, int port, int bg, double width,
                               double height, double pointsize, const Rcpp::List &aliases, bool recording)
    {

        pDevDesc dd = (DevDesc *)calloc(1, sizeof(DevDesc));
        if (dd == nullptr)
        {
            return dd;
        }

        dd->startfill = bg;
        dd->startcol = R_RGB(0, 0, 0);
        dd->startps = pointsize;
        dd->startlty = 0;
        dd->startfont = 1;
        dd->startgamma = 1;

        // Callbacks
        dd->activate = nullptr;
        dd->deactivate = nullptr;
        dd->close = httpgd_close;
        dd->clip = httpgd_clip;
        dd->size = httpgd_size;
        dd->newPage = httpgd_new_page;
        dd->line = httpgd_line;
        dd->text = httpgd_text;
        dd->strWidth = httpgd_strwidth;
        dd->rect = httpgd_rect;
        dd->circle = httpgd_circle;
        dd->polygon = httpgd_polygon;
        dd->polyline = httpgd_polyline;
        dd->path = httpgd_path;
        dd->mode = httpgd_mode;
        dd->metricInfo = httpgd_metric_info;
        dd->cap = nullptr;
        dd->raster = httpgd_raster;

        // UTF-8 support
        dd->wantSymbolUTF8 = (Rboolean)1;
        dd->hasTextUTF8 = (Rboolean)1;
        dd->textUTF8 = httpgd_text;
        dd->strWidthUTF8 = httpgd_strwidth;

        // Screen Dimensions in pts
        dd->left = 0;
        dd->top = 0;
        dd->right = width;
        dd->bottom = height;

        // Magic constants copied from other graphics devices
        // nominal character sizes in pts
        dd->cra[0] = 0.9 * pointsize;
        dd->cra[1] = 1.2 * pointsize;
        // character alignment offsets
        dd->xCharOffset = 0.4900;
        dd->yCharOffset = 0.3333;
        dd->yLineBias = 0.2;
        // inches per pt
        dd->ipr[0] = 1.0 / 72.0;
        dd->ipr[1] = 1.0 / 72.0;

        // Capabilities
        dd->canClip = (Rboolean)1;
        dd->canHAdj = 0;
        dd->canChangeGamma = (Rboolean)0;
        dd->displayListOn = (Rboolean)1; // THIS TOGGLES REPLAYABILITY !!!
        dd->haveTransparency = 2;
        dd->haveTransparentBg = 2;

        dd->deviceSpecific = new HttpgdDev(dd, host, port, aliases, width, height, recording);
        return dd;
    }

    void makehttpgdDevice(const std::string &host, int port, const std::string &bg_, double width, double height,
                          double pointsize, const Rcpp::List &aliases, bool recording)
    {

        int bg = R_GE_str2col(bg_.c_str());

        R_GE_checkVersionOrDie(R_GE_version);
        R_CheckDeviceAvailable();

        HTTPGD_BEGIN_SUSPEND_INTERRUPTS
        {
            if (check_server_started(host, port)) // todo: it should be possible to check if the port is occupied instead
            {
                Rcpp::stop("Failed to start httpgd. Server already running at this address!");
            }

            pDevDesc dev = httpgd_driver_new(host, port, bg, width, height, pointsize, aliases, recording);
            if (dev == nullptr)
            {
                Rcpp::stop("Failed to start httpgd.");
            }

            pGEDevDesc dd = GEcreateDevDesc(dev);
            GEaddDevice2(dd, "httpgd");
            GEinitDisplayList(dd);

            getDev(dev)->server.start();
        }
        HTTPGD_END_SUSPEND_INTERRUPTS;
    }

} // namespace httpgd

// [[Rcpp::export]]
bool httpgd_(Rcpp::String host, int port, Rcpp::String bg, double width, double height,
             double pointsize, Rcpp::List aliases, bool recording)
{
    httpgd::makehttpgdDevice(host, port, bg, width, height, pointsize, aliases, recording);

    return true;
}