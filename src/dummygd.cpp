
#include <Rcpp.h>

#include "devGeneric.h"

namespace httpgd
{

    class devDummy : public devGeneric
    {
    public:
        devDummy() : devGeneric(800, 600, 1) {}

        virtual ~devDummy()
        {
            Rcpp::Rcout << "Device destructed.\n";
        }

    protected:
        virtual void dev_activate(pDevDesc dd) override
        {
            Rcpp::Rcout << "ACTIVATE\n";
        }
        virtual void dev_deactivate(pDevDesc dd) override
        {
            Rcpp::Rcout << "DEACTIVATE\n";
        }
        virtual void dev_close(pDevDesc dd) override
        {
            Rcpp::Rcout << "CLOSE\n";
        }
        virtual void dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd) override
        {
            Rprintf("CLIP x0=%f x1=%f y0=%f y1=%f\n", x0, x1, y0, y1);
        }
        virtual void dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd) override
        {
            Rprintf("SIZE left=%f right=%f bottom=%f top=%f\n", left, right, bottom, top);
        }
        virtual void dev_newPage(pGEcontext gc, pDevDesc dd) override
        {
            Rcpp::Rcout << "NEW_PAGE \n";
        }
        virtual void dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd) override
        {
            Rprintf("LINE x1=%f y1=%f x2=%f y2=%f\n", x1, y1, x2, y2);
        }
        virtual void dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd) override
        {
            Rprintf("TEXT x=%f y=%f str=\"%s\" rot=%f hadj=%f\n", x, y, str, rot, hadj);
        }
        virtual double dev_strWidth(const char *str, pGEcontext gc, pDevDesc dd) override
        {
            Rprintf("STRWIDTH str=\"%s\"\n", str);
            return 1.0;
        }
        virtual void dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd) override
        {

            Rprintf("RECT x0=%f y0=%f x1=%f y1=%f\n", x0, y0, x1, y1);
        }
        virtual void dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd) override
        {
            Rprintf("CIRCLE x=%f y=%f r=%f\n", x, y, r);
        }
        virtual void dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd) override
        {
            Rcpp::Rcout << "POLY \n";
        }
        virtual void dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd) override
        {
            Rprintf("POLYLINE n=%i\n", n);
            for (int i = 0; i < n; i++)
            {
                Rprintf(" [%i] x=%f y=%f\n", i, x[i], y[i]);
            }
        }
        virtual void dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd) override
        {
            Rcpp::Rcout << "PATH \n";
        }
        virtual void dev_mode(int mode, pDevDesc dd) override
        {
            Rcpp::Rcout << "MODE mode=" << mode << "\n";
        }
        virtual void dev_metricInfo(int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd) override
        {
            Rprintf("METRIC_INFO c=%i ascent=%f descent=%f width=%f\n", c, ascent, descent, width);
        }

        virtual SEXP dev_cap(pDevDesc dd) override
        {
            Rcpp::Rcout << "CAP \n";
            return devGeneric::dev_cap(dd);
        }
        virtual void dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd) override
        {
            Rcpp::Rcout << "RASTER \n";
        }
    };

} // namespace httpgd

//using namespace Rcpp;

// --------------------------------------

//' Dummy graphics device for debug purposes.
//'
//' @export
// [[Rcpp::export]]
bool dummygd_()
{
    httpgd::devGeneric::make_device(
        "adummy",
        new httpgd::devDummy());

    return true;
}
