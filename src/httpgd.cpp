
#ifdef _WIN32
#include <winsock2.h>
#endif

#include <Rcpp.h>
#include <R_ext/GraphicsDevice.h>
#include <R_ext/GraphicsEngine.h>
#include <later_api.h>
#include <gdtools.h>
#include "httplib.h"

#include <vector>
#include <mutex>

#include "drawdata.h"
#include "sltools.h"

#define LOGDRAW 0

using namespace httpgd;

//pGEDevDesc httpgd_pGEDevDesc = nullptr;

// returns system path to {package}/inst/www/index.html
std::string get_htmlpath()
{
  Rcpp::Environment base("package:base");
  Rcpp::Function sys_file = base["system.file"];
  Rcpp::StringVector res = sys_file("www", "index.html",
                                    Rcpp::_["package"] = "httpgd");
  return std::string(res[0]);
}

/** 
 * httpgd device data.
 * This runs the server and owns the draw call "DOM".
 */
class HttpgdDev
{
public:
  pDevDesc m_dd;
  Rcpp::List m_system_aliases;
  Rcpp::List m_user_aliases;
  XPtrCairoContext m_cc;
  std::string m_livehtml;

  HttpgdDev(pDevDesc dd, std::string host, int port, Rcpp::List aliases, double width, double height)
      : m_dd(dd), m_system_aliases(Rcpp::wrap(aliases["system"])),
        m_user_aliases(Rcpp::wrap(aliases["user"])),
        m_cc(gdtools::context_create()),
        m_svr_thread(), m_page(width, height), m_host(host), m_port(port)
  {
    // read live server html
    std::ifstream t(get_htmlpath());
    std::stringstream buffer;
    buffer << t.rdbuf();
    m_livehtml = std::string(buffer.str());

    
    m_page.m_fill = dd->startfill;

   
  }
  ~HttpgdDev()
  {
    stop_server();
  }
  void start_server()
  {
    m_svr_thread = std::thread(&HttpgdDev::ThreadMain, this);
  }
  void stop_server()
  {
    m_svr.stop();
    if (m_svr_thread.joinable())
    {
      m_svr_thread.join();
    }
  }
  void page_put(dc::DrawCall *dc)
  {
    m_page_mutex.lock();
    m_page.put(dc);
    m_page_mutex.unlock();
  }
  void page_clear()
  {
    m_page_mutex.lock();
    m_page.clear();
    m_page_mutex.unlock();
  }
  void page_fill(int fill)
  {
    m_page_mutex.lock();
    m_page.m_fill = fill;
    m_page_mutex.unlock();
  }
  void get_svg(std::string &buf)
  {
    m_page_mutex.lock();
    m_page.to_svg(buf);
    m_page_mutex.unlock();
  }
  void page_resize(double w, double h)
  {
    m_page_mutex.lock();
    m_page.m_width = w;
    m_page.m_height = h;
    m_page.clear();
    m_page_mutex.unlock();
  }
  double page_width()
  {
    return m_page.m_width;
  }
  double page_height()
  {
    return m_page.m_height;
  }

private:
  std::thread m_svr_thread;
  httplib::Server m_svr;
  dc::Page m_page;
  std::mutex m_page_mutex;
  std::string m_host;
  int m_port;

  void ThreadMain()
  {
    using namespace httplib;

    m_svr.Get("/", [](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content("httpgd server running.", "text/plain");
    });
    m_svr.Get("/live", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

       // build params
      std::string sparams = std::string("{ host: \"").append(m_host);
      sparams.append("\", port: ").append(std::to_string(m_port));
      m_page_mutex.lock();
      sparams.append(", width: ").append(std::to_string(m_page.m_width));
      sparams.append(", height: ").append(std::to_string(m_page.m_height));
      m_page_mutex.unlock();
      sparams.append(" }");

      // inject params
      std::string html(m_livehtml);
      size_t start_pos = m_livehtml.find("__SRVRPARAMS__");
      if (start_pos != std::string::npos) {
        html.replace(start_pos, sizeof("__SRVRPARAMS__") - 1, sparams);
      }

      res.set_content(html, "text/html");
    });
    m_svr.Get("/svg", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      std::string s;
      s.reserve(1000000);
      this->get_svg(s);
      res.set_content(s, "image/svg+xml");
    });
    m_svr.Get("/check", [this](const Request & /*req*/, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");

      char buffer[200];
      std::sprintf(buffer, "{ \"upid\": %i }", m_page.get_upid());
      res.set_content(buffer, "application/json");
    });
    m_svr.Post("/rs", [this](const Request &req, Response &res) {
      res.set_header("Access-Control-Allow-Origin", "*");
      Params par;
      detail::parse_query_text(req.body, par);

      double w = m_page.m_width;
      double h = m_page.m_height;

      for (const auto &e : par)
      {
        if (e.first == "w")
        {
          try
          {
            w = std::stod(e.second);
          }
          catch (const std::exception &e)
          {
          }
        }
        else if (e.first == "h")
        {
          try
          {
            h = std::stod(e.second);
          }
          catch (const std::exception &e)
          {
          }
        }
      }

      if (w != m_page.m_width || h != m_page.m_height)
      {
        page_resize(w, h);
        m_dd->size(&(m_dd->left), &(m_dd->right), &(m_dd->bottom), &(m_dd->top), m_dd);

        later::later([](void *dd) { GEplayDisplayList(desc2GEDesc((pDevDesc)dd)); }, m_dd, 1.0);
      }

      res.set_content("{ \"status\": \"ok\" }", "application/json");
    });

    //m_svr.Get("/stop",
    //        [&](const Request & /*req*/, Response & /*res*/) { m_svr.stop(); });

    m_svr.listen(m_host.c_str(), m_port);
  }
};

inline HttpgdDev *getDev(pDevDesc dd)
{
  return (HttpgdDev *)dd->deviceSpecific;
}

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
  // todo

#if LOGDRAW == 1
  Rprintf("CLIP x0=%f x1=%f y0=%f y1=%f\n", x0, x1, y0, y1);
#endif
}

/**
 * R Callback: Start new page.
 */
void httpgd_new_page(const pGEcontext gc, pDevDesc dd)
{
  getDev(dd)->page_clear();
  getDev(dd)->page_fill(dd->startfill); // todo should this be gc->fill ?

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

  HttpgdDev *svgd = getDev(dd);
  svgd->stop_server();
  free(svgd);

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
  getDev(dd)->page_put(dc);

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
  getDev(dd)->page_put(dc);

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
  getDev(dd)->page_put(dc);

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
  for (int i = 0; i < npoly; i++) {
    npoints += vnper[i];
  }
  std::vector<double> vx(x, x + npoints);
  std::vector<double> vy(y, y + npoints);

  dc::Path *dc = new dc::Path(vx,vy, npoly, vnper, winding);
  copyGc(gc, dc);
  getDev(dd)->page_put(dc);

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
  getDev(dd)->page_put(dc);

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
  getDev(dd)->page_put(dc);

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

  dev->page_put(dc);
}

/**
 * R Callback: Get size of drawing.
 */
void httpgd_size(double *left, double *right, double *bottom, double *top,
                 pDevDesc dd)
{
  HttpgdDev *dev = getDev(dd);

  *left = 0.0;
  *right = dev->page_width();
  *bottom = dev->page_height();
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

  std::vector<unsigned int> raster_(raster, raster + (w*h));

  dc::Raster *dc = new dc::Raster(raster_,w,h,x,y,width,height,rot,interpolate);
  copyGc(gc, dc);
  getDev(dd)->page_put(dc);

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
                           double height, double pointsize, Rcpp::List &aliases)
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

  R_ProcessEvents();

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
  dd->canClip = (Rboolean)0;
  dd->canHAdj = 0;
  dd->canChangeGamma = (Rboolean)0;
  dd->displayListOn = (Rboolean)1; // THIS TOGGLES REPLAYABILITY !!!
  dd->haveTransparency = 2;
  dd->haveTransparentBg = 2;

  dd->deviceSpecific = new HttpgdDev(dd, host, port, aliases, width, height);
  return dd;
}

void makehttpgdDevice(std::string host, int port, std::string bg_, double width, double height,
                      double pointsize, Rcpp::List &aliases)
{

  int bg = R_GE_str2col(bg_.c_str());

  R_GE_checkVersionOrDie(R_GE_version);
  R_CheckDeviceAvailable();

  pDevDesc dev;

  BEGIN_SUSPEND_INTERRUPTS
  {

    dev = httpgd_driver_new(host, port, bg, width, height, pointsize, aliases);
    if (dev == NULL)
      Rcpp::stop("Failed to start httpgd.");

    pGEDevDesc dd = GEcreateDevDesc(dev);
    GEaddDevice2(dd, "httpgd");
    GEinitDisplayList(dd);
  }
  END_SUSPEND_INTERRUPTS;

  getDev(dev)->start_server();
}

// [[Rcpp::export]]
bool httpgd_(Rcpp::String host, int port, std::string bg, double width, double height,
             double pointsize, Rcpp::List aliases)
{
  makehttpgdDevice(host, port, bg, width, height, pointsize, aliases);

  return true;
}
