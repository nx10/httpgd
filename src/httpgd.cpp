// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>
#include <later_api.h>
#include <gdtools.h>

#include <vector>
#include <mutex>

#include "drawdata.h"
#include "sltools.h"
#include "HttpgdServer.h"
#include "HttpgdDev.h"

#include "fixsuspinter.h"

#define LOGDRAW 0

static void hist_play(pDevDesc dd);
static void hist_clear(pDevDesc dd);

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

  inline HttpgdDev *getDev(pDevDesc dd)
  {
    return (HttpgdDev *)dd->deviceSpecific;
  }

  HttpgdDev::HttpgdDev(pDevDesc dd, std::string host, int port, Rcpp::List aliases, double width, double height, bool recording)
      : m_dd(dd), m_system_aliases(Rcpp::wrap(aliases["system"])),
        m_user_aliases(Rcpp::wrap(aliases["user"])),
        m_cc(gdtools::context_create()),
        m_server(host, port, width, height, recording),
        m_history(8927, 4, std::string(".httpgdPlots_").append(std::to_string(port))),
        m_replaying(false), m_needsave(false)
  {
    // setup callbacks
    m_server.m_user_resized = [&]() { user_resized(); };
    //m_server.m_user_hist_record = [&](bool recording){ user_hist_record(recording); };
    m_server.m_user_hist_play = [&]() { user_hist_play(); };
    m_server.m_user_hist_clear = [&]() { user_hist_clear(); };

    // read live server html
    std::ifstream t(get_wwwpath("index.html"));
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string livehtml = std::string(buffer.str());
    m_server.set_livehtml(livehtml);

    // set start fill
    m_server.page_fill(dd->startfill);
  }

  HttpgdDev::~HttpgdDev()
  {
  }

  void HttpgdDev::user_resized()
  {
    m_replaying = true;
    later::later([](void *ddp) {
      pDevDesc dd = (pDevDesc)ddp;
      dd->size(&(dd->left), &(dd->right), &(dd->bottom), &(dd->top), dd);
      GEplayDisplayList(desc2GEDesc(dd));
      getDev(dd)->m_replaying = false;
    },
                 m_dd, 0.0);
  }

  void HttpgdDev::user_hist_play()
  {
    later::later([](void *ddp) {
      pDevDesc dd = (pDevDesc)ddp;
      hist_play(dd);
    },
                 m_dd, 0.0);
  }
  void HttpgdDev::user_hist_clear()
  {
    later::later([](void *ddp) {
      pDevDesc dd = (pDevDesc)ddp;
      hist_clear(dd);
    },
                 m_dd, 0.0);
  }

} // namespace httpgd

using namespace httpgd;

// plot history

static void hist_update_size(pDevDesc dd)
{
  HttpgdDev *xd = (HttpgdDev *)dd->deviceSpecific;
  int history_size = xd->m_history.size();
  if (xd->m_needsave)
  {
    history_size += 1;
  }
  xd->m_server.set_history_size(history_size);
}

static void hist_play(pDevDesc dd)
{
  HttpgdDev *xd = (HttpgdDev *)dd->deviceSpecific;

  if (xd->m_server.is_recording() && xd->m_needsave)
    {
      xd->m_history.push_current(dd);
      xd->m_needsave = false;
    }

  int index = xd->m_server.get_history_index();
  xd->m_replaying = true;
  xd->m_history.play(index, dd);
  xd->m_replaying = false;

  hist_update_size(dd);
}


static void hist_clear(pDevDesc dd)
{
  HttpgdDev *xd = (HttpgdDev *)dd->deviceSpecific;
  xd->m_history.clear();
  xd->m_needsave = false;
  xd->m_server.set_history_size(0);
}

/* end of plot history */

// -------------------

/**
 * Copy (stateful) graphics parameters.
 * Font information is not copied.
 */
void copyGc(const pGEcontext gc, dc::DrawCall *dc)
{
  dc->m_col = gc->col;
  dc->m_fill = gc->fill;
  dc->m_gamma = gc->gamma;
  dc->m_lwd = gc->lwd;
  dc->m_lty = gc->lty;
  dc->m_lend = (dc::GC_lineend)gc->lend;
  dc->m_ljoin = (dc::GC_linejoin)gc->ljoin;
  dc->m_lmitre = gc->lmitre;
}

// --------------------------------------

/**
 * R Callback: Get singe char font metrics.
 */
void httpgd_metric_info(int c, const pGEcontext gc, double *ascent,
                        double *descent, double *width, pDevDesc dd)
{
  HttpgdDev *svgd = getDev(dd);

  bool is_unicode = mbcslocale;
  if (c < 0)
  {
    is_unicode = true;
    c = -c;
  }

  // Convert to string - negative implies unicode code point
  char str[16];
  if (is_unicode)
  {
    Rf_ucstoutf8(str, (unsigned int)c);
  }
  else
  {
    str[0] = (char)c;
    str[1] = '\0';
  }

  std::string file = fontfile(gc->fontfamily, gc->fontface, svgd->m_user_aliases);
  std::string name = fontname(gc->fontfamily, gc->fontface, svgd->m_system_aliases, svgd->m_user_aliases);
  gdtools::context_set_font(svgd->m_cc, name, gc->cex * gc->ps, is_bold(gc->fontface), is_italic(gc->fontface), file);
  FontMetric fm = gdtools::context_extents(svgd->m_cc, std::string(str));

  *ascent = fm.ascent;
  *descent = fm.descent;
  *width = fm.width;

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

  HttpgdDev *svgd = getDev(dd);

  std::string file = fontfile(gc->fontfamily, gc->fontface, svgd->m_user_aliases);
  std::string name = fontname(gc->fontfamily, gc->fontface, svgd->m_system_aliases, svgd->m_user_aliases);
  gdtools::context_set_font(svgd->m_cc, name, gc->cex * gc->ps, is_bold(gc->fontface), is_italic(gc->fontface), file);
  FontMetric fm = gdtools::context_extents(svgd->m_cc, std::string(str));

  return fm.width;
}

/**
 * R Callback: Clip draw area.
 */
void httpgd_clip(double x0, double x1, double y0, double y1, pDevDesc dd)
{
  getDev(dd)->m_server.page_clip(x0, x1, y0, y1);
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

  if (dev->m_server.is_recording() && dev->m_needsave)
  {
    dev->m_history.push_last(dd);
  }
  dev->m_needsave = !dev->m_replaying;
  if (!dev->m_replaying) {
    hist_update_size(dd);
  }

  dev->m_server.page_clear();
  dev->m_server.page_fill(dd->startfill); // todo should this be gc->fill ?

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
  dev->m_history.clear();
  dev->m_server.stop();
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
  dc::Line *dc = new dc::Line(x1, y1, x2, y2);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

  dc::Polyline *dc = new dc::Polyline(n, vx, vy);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

  dc::Polygon *dc = new dc::Polygon(n, vx, vy);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

  dc::Path *dc = new dc::Path(vx, vy, npoly, vnper, winding);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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
  dc::Rect *dc = new dc::Rect(x0, y0, x1, y1);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

  dc::Circle *dc = new dc::Circle(x, y, r);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

  dc::Text *dc = new dc::Text(x, y, str, rot, hadj);
  copyGc(gc, dc);

  dc->m_fontsize = gc->cex * gc->ps;
  dc->m_bold = is_bold(gc->fontface);
  dc->m_italic = is_italic(gc->fontface);

  dc->m_font_family = fontname(gc->fontfamily, gc->fontface, dev->m_system_aliases, dev->m_user_aliases);
  std::string file = fontfile(gc->fontfamily, gc->fontface, dev->m_user_aliases);

  gdtools::context_set_font(dev->m_cc, dc->m_font_family, dc->m_fontsize, is_bold(gc->fontface), is_italic(gc->fontface), file);
  FontMetric fm = gdtools::context_extents(dev->m_cc, std::string(str));
  dc->m_txtwidth_px = fm.width;

#if LOGDRAW == 1
  Rprintf("TEXT x=%f y=%f str=\"%s\" rot=%f hadj=%f\n", x, y, str, rot, hadj);
#endif

  dev->m_server.page_put(dc);
}

/**
 * R Callback: Get size of drawing.
 */
void httpgd_size(double *left, double *right, double *bottom, double *top,
                 pDevDesc dd)
{
  HttpgdDev *dev = getDev(dd);

  *left = 0.0;
  *right = dev->m_server.page_width();
  *bottom = dev->m_server.page_height();
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

  dc::Raster *dc = new dc::Raster(raster_, w, h, x, y, width, height, rot, interpolate);
  copyGc(gc, dc);
  getDev(dd)->m_server.page_put(dc);

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

pDevDesc httpgd_driver_new(std::string host, int port, int bg, double width,
                           double height, double pointsize, Rcpp::List &aliases, bool recording)
{

  pDevDesc dd = (DevDesc *)calloc(1, sizeof(DevDesc));
  if (dd == NULL)
    return dd;

  dd->startfill = bg;
  dd->startcol = R_RGB(0, 0, 0);
  dd->startps = pointsize;
  dd->startlty = 0;
  dd->startfont = 1;
  dd->startgamma = 1;

  // Callbacks
  dd->activate = NULL;
  dd->deactivate = NULL;
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
  dd->cap = NULL;
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

void makehttpgdDevice(std::string host, int port, std::string bg_, double width, double height,
                      double pointsize, Rcpp::List &aliases, bool recording)
{

  int bg = R_GE_str2col(bg_.c_str());

  R_GE_checkVersionOrDie(R_GE_version);
  R_CheckDeviceAvailable();

  pDevDesc dev;

  HTTPGD_BEGIN_SUSPEND_INTERRUPTS
  {

    dev = httpgd_driver_new(host, port, bg, width, height, pointsize, aliases, recording);
    if (dev == NULL)
      Rcpp::stop("Failed to start httpgd.");

    pGEDevDesc dd = GEcreateDevDesc(dev);
    GEaddDevice2(dd, "httpgd");
    GEinitDisplayList(dd);
  }
  HTTPGD_END_SUSPEND_INTERRUPTS;

  getDev(dev)->m_server.start();
}

// [[Rcpp::export]]
bool httpgd_(Rcpp::String host, int port, std::string bg, double width, double height,
             double pointsize, Rcpp::List aliases, bool recording)
{
  makehttpgdDevice(host, port, bg, width, height, pointsize, aliases, recording);

  return true;
}

// namespace httpgd