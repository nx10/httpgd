
#include <Rcpp.h>

//using namespace Rcpp;

// --------------------------------------


void dummy_metric_info(int c, const pGEcontext gc, double* ascent,
                     double* descent, double* width, pDevDesc dd) {
  //SVGDesc *svgd = (SVGDesc*) dd->deviceSpecific;
  
  Rprintf("METRIC_INFO c=%i ascent=%f descent=%f width=%f\n", c, ascent, descent, width);
  
}

void dummy_clip(double x0, double x1, double y0, double y1, pDevDesc dd) {
  
  Rprintf("CLIP x0=%f x1=%f y0=%f y1=%f\n", x0, x1, y0, y1);
}

void dummy_new_page(const pGEcontext gc, pDevDesc dd) {
  Rcpp::Rcout << "NEW_PAGE \n";
}

void dummy_close(pDevDesc dd) {
  Rcpp::Rcout << "CLOSE \n";
}

void dummy_line(double x1, double y1, double x2, double y2,
              const pGEcontext gc, pDevDesc dd) {
  
  
  Rprintf("LINE x1=%f y1=%f x2=%f y2=%f\n", x1, y1, x2, y2);
}

void dummy_poly(int n, double *x, double *y, int filled, const pGEcontext gc,
              pDevDesc dd, const char* node_name) {
  
  Rcpp::Rcout << "POLY \n";
  
}

void dummy_polyline(int n, double *x, double *y, const pGEcontext gc,
                  pDevDesc dd) {
  
  Rprintf("POLYLINE n=%i\n", n);
  for (int i = 0; i < n; i++) {
    Rprintf(" [%i] x=%f y=%f\n", i, x[i], y[i]);
  }
}
void dummy_polygon(int n, double *x, double *y, const pGEcontext gc,
                 pDevDesc dd) {
  Rcpp::Rcout << "POLYGON \n";
}

void dummy_path(double *x, double *y,
              int npoly, int *nper,
              Rboolean winding,
              const pGEcontext gc, pDevDesc dd) {
  Rcpp::Rcout << "PATH \n";
}

double dummy_strwidth(const char *str, const pGEcontext gc, pDevDesc dd) {
  
  Rprintf("STRWIDTH str=\"%s\"\n", str);
  
  return 1.0;
}

void dummy_rect(double x0, double y0, double x1, double y1,
              const pGEcontext gc, pDevDesc dd) {
  
  Rprintf("RECT x0=%f y0=%f x1=%f y1=%f\n", x0, y0, x1, y1);
}

void dummy_circle(double x, double y, double r, const pGEcontext gc,
                pDevDesc dd) {
  Rprintf("CIRCLE x=%f y=%f r=%f\n", x, y, r);
}

void dummy_text(double x, double y, const char *str, double rot,
              double hadj, const pGEcontext gc, pDevDesc dd) {
  
  Rprintf("TEXT x=%f y=%f str=\"%s\" rot=%f hadj=%f\n", x, y, str, rot, hadj);
}

void dummy_size(double *left, double *right, double *bottom, double *top,
              pDevDesc dd) {
  
  
  *left = dd->left;
  *right = dd->right;
  *bottom = dd->bottom;
  *top = dd->top;
  
  Rprintf("SIZE left=%f right=%f bottom=%f top=%f\n", left, right, bottom, top);
}

void dummy_raster(unsigned int *raster, int w, int h,
                double x, double y,
                double width, double height,
                double rot,
                Rboolean interpolate,
                const pGEcontext gc, pDevDesc dd) {
  Rcpp::Rcout << "RASTER \n";
}



// --------------------------------------

pDevDesc dummy_driver_new() {
  
  double width = 60.0;
  double height = 40.0;
  double pointsize = 1.0; // ?
  
  pDevDesc dd = (DevDesc*) calloc(1, sizeof(DevDesc));
  if (dd == NULL)
    return dd;
  
  dd->startfill = R_RGB(255, 255, 255);
  dd->startcol = R_RGB(0, 0, 0);
  dd->startps = pointsize;
  dd->startlty = 0;
  dd->startfont = 1;
  dd->startgamma = 1;
  
  // Callbacks
  dd->activate = NULL;
  dd->deactivate = NULL;
  dd->close = dummy_close;
  dd->clip = dummy_clip;
  dd->size = dummy_size;
  dd->newPage = dummy_new_page;
  dd->line = dummy_line;
  dd->text = dummy_text;
  dd->strWidth = dummy_strwidth;
  dd->rect = dummy_rect;
  dd->circle = dummy_circle;
  dd->polygon = dummy_polygon;
  dd->polyline = dummy_polyline;
  dd->path = dummy_path;
  dd->mode = NULL;
  dd->metricInfo = dummy_metric_info;
  dd->cap = NULL;
  dd->raster = dummy_raster;
  
  // UTF-8 support
  dd->wantSymbolUTF8 = (Rboolean) 1;
  dd->hasTextUTF8 = (Rboolean) 1;
  dd->textUTF8 = dummy_text;
  dd->strWidthUTF8 = dummy_strwidth;
  
  // Screen Dimensions in pts
  dd->left = 0;
  dd->top = 0;
  dd->right = width * 72;
  dd->bottom = height * 72;
  
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
  dd->canClip = (Rboolean) TRUE;
  dd->canHAdj = 0;
  dd->canChangeGamma = (Rboolean) FALSE;
  dd->displayListOn = (Rboolean) FALSE;
  dd->haveTransparency = 2;
  dd->haveTransparentBg = 2;
  
  dd->deviceSpecific = NULL;//new SVGDesc(stream, standalone, aliases);
  return dd;
}

void makeDummyDevice() {
  
  //int bg = R_GE_str2col(bg_.c_str());
  
  R_GE_checkVersionOrDie(R_GE_version);
  R_CheckDeviceAvailable();
  
  //BEGIN_SUSPEND_INTERRUPTS {
  pDevDesc dev = dummy_driver_new();
  if (dev == NULL)
    Rcpp::stop("Failed to start dummy device");
  
  pGEDevDesc dd = GEcreateDevDesc(dev);
  GEaddDevice2(dd, "devDummy");
  GEinitDisplayList(dd);
  
  //} END_SUSPEND_INTERRUPTS;
}

// [[Rcpp::export]]
bool dummygd_() {
  
  makeDummyDevice();
  
  return true;
}
