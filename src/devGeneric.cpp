
#include "devGeneric.h"
#include <cpp11/protect.hpp> // for cpp11::stop

namespace httpgd
{
    devGeneric::devGeneric(double t_width, double t_height, double t_pointsize, int t_fill)
        : m_initial_width(t_width),
          m_initial_height(t_height),
          m_initial_pointsize(t_pointsize),
          m_initial_fill(t_fill)
    {
    }

    inline devGeneric *getDev(pDevDesc dd)
    {
        return static_cast<devGeneric *>(dd->deviceSpecific);
    }

    pDevDesc devGeneric::create()
    {
        pDevDesc dd = new DevDesc();
        if (dd == nullptr)
            return dd;

        dd->startfill = m_initial_fill;
        dd->startcol = m_initial_col;
        dd->startps = m_initial_pointsize;
        dd->startlty = 0;
        dd->startfont = 1;
        dd->startgamma = 1;

        // Callbacks
        dd->activate = [](pDevDesc dd) { getDev(dd)->dev_activate(dd); };
        dd->deactivate = [](pDevDesc dd) { getDev(dd)->dev_deactivate(dd); };
        dd->close = [](pDevDesc dd) {
            getDev(dd)->dev_close(dd);
            delete getDev(dd);
        };
        dd->clip = [](double x0, double x1, double y0, double y1, pDevDesc dd) { getDev(dd)->dev_clip(x0, x1, y0, y1, dd); };
        dd->size = [](double *left, double *right, double *bottom, double *top, pDevDesc dd) {
            *left = dd->left;
            *top = dd->top;
            *right = dd->right;
            *bottom = dd->bottom;

            getDev(dd)->dev_size(left, right, bottom, top, dd);
        };
        dd->newPage = [](pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_newPage(gc, dd); };
        dd->line = [](double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_line(x1, y1, x2, y2, gc, dd); };
        dd->text = [](double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_text(x, y, str, rot, hadj, gc, dd); };
        dd->strWidth = [](const char *str, pGEcontext gc, pDevDesc dd) { return getDev(dd)->dev_strWidth(str, gc, dd); };
        dd->rect = [](double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_rect(x0, y0, x1, y1, gc, dd); };
        dd->circle = [](double x, double y, double r, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_circle(x, y, r, gc, dd); };
        dd->polygon = [](int n, double *x, double *y, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_polygon(n, x, y, gc, dd); };
        dd->polyline = [](int n, double *x, double *y, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_polyline(n, x, y, gc, dd); };
        dd->path = [](double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_path(x, y, npoly, nper, winding, gc, dd); };
        dd->mode = [](int mode, pDevDesc dd) { getDev(dd)->dev_mode(mode, dd); };
        dd->metricInfo = [](int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd) { getDev(dd)->dev_metricInfo(c, gc, ascent, descent, width, dd); };
        dd->raster = [](unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd) { getDev(dd)->dev_raster(raster, w, h, x, y, width, height, rot, interpolate, gc, dd); };

        if (m_df_cap)
        {
            dd->cap = [](pDevDesc dd) { return getDev(dd)->dev_cap(dd); };
        }
        else
        {
            dd->cap = nullptr;
        }

        // UTF-8 support
        dd->wantSymbolUTF8 = static_cast<Rboolean>(1);
        dd->hasTextUTF8 = static_cast<Rboolean>(1);
        dd->textUTF8 = dd->text;
        dd->strWidthUTF8 = dd->strWidth;

        // Screen Dimensions in pts
        dd->left = 0;
        dd->top = 0;
        dd->right = m_initial_width;
        dd->bottom = m_initial_height;

        // Magic constants copied from other graphics devices
        // nominal character sizes in pts
        dd->cra[0] = 0.9 * m_initial_pointsize;
        dd->cra[1] = 1.2 * m_initial_pointsize;
        // character alignment offsets
        dd->xCharOffset = 0.4900;
        dd->yCharOffset = 0.3333;
        dd->yLineBias = 0.2;
        // inches per pt
        dd->ipr[0] = 1.0 / 72.0;
        dd->ipr[1] = 1.0 / 72.0;

        // Capabilities
        dd->canClip = static_cast<Rboolean>(1);
        dd->canHAdj = 1;
        dd->canChangeGamma = static_cast<Rboolean>(0);
        dd->displayListOn = static_cast<Rboolean>(m_df_displaylist);
        dd->haveTransparency = 2;
        dd->haveTransparentBg = 3;

        dd->haveRaster = 2;
        dd->haveCapture = 1;
        dd->haveLocator = 1;

        dd->newFrameConfirm = nullptr;
        dd->onExit = nullptr;
        dd->eventEnv = R_NilValue;
        dd->eventHelper = nullptr;
        dd->holdflush = nullptr;

#if R_GE_version >= 13
        dd->deviceVersion = R_GE_definitions;
        dd->canClip = static_cast<Rboolean>(0);
#endif

        // Device specific
        dd->deviceSpecific = this;

        return dd;
    }

    void devGeneric::make_device(const char *t_device_name, devGeneric *t_dev)
    {

        R_GE_checkVersionOrDie(R_GE_version);
        R_CheckDeviceAvailable();

        BEGIN_SUSPEND_INTERRUPTS
        {
            pDevDesc dd = t_dev->create();
            if (dd == nullptr)
                cpp11::stop("Failed to start device");

            pGEDevDesc gdd = GEcreateDevDesc(dd);
            GEaddDevice2(gdd, t_device_name);
            GEinitDisplayList(gdd);
        }
        END_SUSPEND_INTERRUPTS;
    }

    pDevDesc devGeneric::get_active_pDevDesc()
    {
        pGEDevDesc gdd = GEcurrentDevice();
        if (gdd == nullptr)
            cpp11::stop("Current device not found");
        pDevDesc dd = gdd->dev;
        if (dd == nullptr)
            cpp11::stop("Current device not found");
        return dd;
    }

    // CALLBACKS

    void devGeneric::dev_activate(pDevDesc dd)
    {
    }
    void devGeneric::dev_deactivate(pDevDesc dd)
    {
    }
    void devGeneric::dev_close(pDevDesc dd)
    {
    }
    void devGeneric::dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd)
    {
    }
    void devGeneric::dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd)
    {
    }
    void devGeneric::dev_newPage(pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd)
    {
    }
    double devGeneric::dev_strWidth(const char *str, pGEcontext gc, pDevDesc dd)
    {
        return 0;
    }
    void devGeneric::dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd)
    {
    }
    void devGeneric::dev_mode(int mode, pDevDesc dd)
    {
    }
    void devGeneric::dev_metricInfo(int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd)
    {
    }
    SEXP devGeneric::dev_cap(pDevDesc dd)
    {
        return R_NilValue;
    }
    void devGeneric::dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd)
    {
    }
#if R_GE_version >= 13
    SEXP devGeneric::dev_setPattern(SEXP pattern, pDevDesc dd)
    {
        return R_NilValue;
    }
    void devGeneric::dev_releasePattern(SEXP ref, pDevDesc dd)
    {
    }
    SEXP devGeneric::dev_setClipPath(SEXP path, SEXP ref, pDevDesc dd)
    {
        return R_NilValue;
    }
    void devGeneric::dev_releaseClipPath(SEXP ref, pDevDesc dd)
    {
    }
    SEXP devGeneric::dev_setMask(SEXP path, SEXP ref, pDevDesc dd)
    {
        return R_NilValue;
    }
    void devGeneric::dev_releaseMask(SEXP ref, pDevDesc dd)
    {
    }
#endif

} // namespace httpgd