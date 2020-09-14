#ifndef HTTPGD_RGRAPHICSENGINECPP_H
#define HTTPGD_RGRAPHICSENGINECPP_H

#include <R_ext/GraphicsEngine.h>

namespace httpgd
{
    class devGeneric
    {
    public:

        devGeneric(double t_width, double t_height, double t_pointsize);
        virtual ~devGeneric() = default;
        
        static void make_device(const char *t_device_name, devGeneric *t_dev);
        // avoid when possible
        static pDevDesc get_active_pDevDesc();
        // Replay the current graphics device state
        static void replay_current(pDevDesc dd);

    protected:
        // DEVICE CALLBACKS

        // Called when the device becomes the active device
        virtual void dev_activate(pDevDesc dd);
        // Called when another device becomes the active device
        virtual void dev_deactivate(pDevDesc dd);
        // Called when the device is closed (Object will be destroyed afterwards)
        virtual void dev_close(pDevDesc dd);
        // Clip draw area
        virtual void dev_clip(double x0, double x1, double y0, double y1, pDevDesc dd);
        // Get the size of the graphics device
        virtual void dev_size(double *left, double *right, double *bottom, double *top, pDevDesc dd);
        // Start a new page
        virtual void dev_newPage(pGEcontext gc, pDevDesc dd);
        // Draw line
        virtual void dev_line(double x1, double y1, double x2, double y2, pGEcontext gc, pDevDesc dd);
        // Draw text
        virtual void dev_text(double x, double y, const char *str, double rot, double hadj, pGEcontext gc, pDevDesc dd);
        // Get String width
        virtual double dev_strWidth(const char *str, pGEcontext gc, pDevDesc dd);
        // Draw rectangle
        virtual void dev_rect(double x0, double y0, double x1, double y1, pGEcontext gc, pDevDesc dd);
        // Draw circle
        virtual void dev_circle(double x, double y, double r, pGEcontext gc, pDevDesc dd);
        // Draw polygon
        virtual void dev_polygon(int n, double *x, double *y, pGEcontext gc, pDevDesc dd);
        // Draw polyline
        virtual void dev_polyline(int n, double *x, double *y, pGEcontext gc, pDevDesc dd);
        // Draw path
        virtual void dev_path(double *x, double *y, int npoly, int *nper, Rboolean winding, pGEcontext gc, pDevDesc dd);
        // start draw mode = 1, stop draw mode = 0
        virtual void dev_mode(int mode, pDevDesc dd);
        // Get singe char font metrics
        virtual void dev_metricInfo(int c, pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd);
        // Integer matrix (R colors)
        virtual SEXP dev_cap(pDevDesc dd);
        // Draw raster image
        virtual void dev_raster(unsigned int *raster, int w, int h, double x, double y, double width, double height, double rot, Rboolean interpolate, pGEcontext gc, pDevDesc dd);

        // GRAPHICS DEVICE FEATURE FLAGS

        bool m_df_cap = false;
        bool m_df_displaylist = false;

        // INITIAL VALUES

        const double m_initial_width;
        const double m_initial_height;
        const double m_initial_pointsize;
        const int m_initial_fill = R_RGB(255, 255, 255);
        const int m_initial_col = R_RGB(0, 0, 0);

    private:
        pDevDesc create();

    }; // namespace httpgd
} // namespace httpgd

#endif