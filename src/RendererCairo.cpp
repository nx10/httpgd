#include "RendererCairo.h"

#include <boost/math/constants/constants.hpp>

//#include <cairo-svg.h>

// https://github.com/wch/r-source/blob/trunk/src/library/grDevices/src/cairo/cairoFns.c
// https://github.com/s-u/Cairo/blob/master/src/cairotalk.c

namespace httpgd::dc
{
    constexpr double MATH_PI { boost::math::constants::pi<double>() };

    /*static cairo_status_t cairowrite_fmt(void *closure, unsigned char const *data, unsigned int length) {
        auto *os = static_cast<fmt::memory_buffer *>(closure);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        std::string s( reinterpret_cast<char const*>(data), length ) ;
        fmt::format_to(*os, "{}", s);
        return CAIRO_STATUS_SUCCESS;
    }*/

    static cairo_status_t cairowrite_png(void *closure, unsigned char const *data, unsigned int length)
    {
        auto *render_data = static_cast<std::vector<unsigned char> *>(closure);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        render_data->insert(render_data->end(), data, data + length); 
        return CAIRO_STATUS_SUCCESS;
    }
    
    /*std::string RendererCairo::to_string() 
    {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return fmt::to_string(os);
    }*/

    inline void set_color(cairo_t *cr, color_t col)
    {
        color_t alpha = color::alpha(col);
        double red = color::red_frac(col);
        double green = color::green_frac(col);
        double blue = color::blue_frac(col);

        /* This optimization should not be necessary, but alpha = 1 seems
       to cause image fallback in some backends */
        if (alpha == color::byte_mask)
        {
            cairo_set_source_rgb(cr, red, green, blue);
        }
        else
        {
            cairo_set_source_rgba(cr, red, green, blue, color::byte_frac(alpha));
        }
    }

    inline void set_linetype(cairo_t *cr, const LineInfo &line)
    {
        cairo_line_cap_t lcap = CAIRO_LINE_CAP_SQUARE;
        cairo_line_join_t ljoin = CAIRO_LINE_JOIN_ROUND;
        switch (line.lend)
        {
        case LineInfo::GC_ROUND_CAP:
            lcap = CAIRO_LINE_CAP_ROUND;
            break;
        case LineInfo::GC_BUTT_CAP:
            lcap = CAIRO_LINE_CAP_BUTT;
            break;
        case LineInfo::GC_SQUARE_CAP:
            lcap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
        switch (line.ljoin)
        {
        case LineInfo::GC_ROUND_JOIN:
            ljoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case LineInfo::GC_MITRE_JOIN:
            ljoin = CAIRO_LINE_JOIN_MITER;
            break;
        case LineInfo::GC_BEVEL_JOIN:
            ljoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        }
        cairo_set_line_width(cr, (line.lwd > 0.01 ? line.lwd : 0.01) / 96.0 * 72);
        cairo_set_line_cap(cr, lcap);
        cairo_set_line_join(cr, ljoin);
        cairo_set_miter_limit(cr, line.lmitre);

        if (line.lty == LineInfo::TY_BLANK || line.lty == dc::LineInfo::TY_SOLID)
            cairo_set_dash(cr, 0, 0, 0);
        else
        {
            double ls[16], lwd = (line.lwd > 1) ? line.lwd : 1;
            int l;
            /* Use unsigned int otherwise right shift of 'dt'
           may not terminate for loop */
            unsigned int dt = line.lty;
            for (l = 0; dt != 0; dt >>= 4, l++)
                ls[l] = (dt & 0xF) * lwd / 96.0 * 72;
            cairo_set_dash(cr, ls, l, 0);
        }
    }

    void RendererCairo::render(const Page &t_page)
    {

        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, t_page.size.x, t_page.size.y);
        //surface = cairo_svg_surface_create("svgfile.svg", 390, 60);
        //surface = cairo_svg_surface_create_for_stream(cairowrite_fmt, &os, 390, 60); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

        cr = cairo_create(surface);

        for (const auto &dc : t_page.dcs)
        {
            dc->render(this);
        }

        //fmt::format_to(os, "blub");

        cairo_surface_write_to_png_stream(surface, cairowrite_png, &m_render_data);

        /*img = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
            0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
            0x08, 0x02, 0x00, 0x00, 0x00, 0xFD, 0xD4, 0x9A, 0x73, 0x00, 0x00, 0x00,
            0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00,
            0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8F, 0x0B, 0xFC,
            0x61, 0x05, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00,
            0x0E, 0xC3, 0x00, 0x00, 0x0E, 0xC3, 0x01, 0xC7, 0x6F, 0xA8, 0x64, 0x00,
            0x00, 0x00, 0x15, 0x49, 0x44, 0x41, 0x54, 0x18, 0x57, 0x63, 0xFE, 0xCF,
            0xC0, 0x70, 0x16, 0x88, 0xFF, 0xFF, 0xFF, 0xCF, 0xA0, 0xF6, 0x1F, 0x00,
            0x2E, 0x60, 0x06, 0xF1, 0x89, 0xAD, 0x3B, 0x21, 0x00, 0x00, 0x00, 0x00,
            0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
        };*/

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }
    
    std::vector<unsigned char> RendererCairo::get() const 
    {
        return m_render_data;
    }

    void RendererCairo::dc(const DrawCall &)
    {
    }

    void RendererCairo::rect(const Rect &t_rect)
    {
        //cairo_rectangle(cr, t_rect.rect.x, t_rect.rect.y, t_rect.rect.width, t_rect.rect.height);
    }

    void RendererCairo::text(const Text &t_text)
    {
    }

    void RendererCairo::circle(const Circle &t_circle)
    {
        cairo_arc(cr, t_circle.pos.x, t_circle.pos.y, (t_circle.radius > 0.5 ? t_circle.radius : 0.5), 0.0, 2 * MATH_PI );

        set_color(cr, t_circle.line.col);
        set_linetype(cr, t_circle.line);
        cairo_stroke(cr);
    }

    void RendererCairo::line(const Line &t_line)
    {
        if (color::alpha(t_line.line.col) == 0)
            return;

        set_color(cr, t_line.line.col);
        set_linetype(cr, t_line.line);

        /*    if (!xd->appending)
        {
            if (xd->currentMask >= 0)
            {
                /* If masking, draw temporary pattern *s/
                cairo_push_group(xd->cc);
            }
            CairoColor(gc->col, xd);
            CairoLineType(gc, xd);
            cairo_new_path(xd->cc);
        }*/

        cairo_move_to(cr, t_line.orig.x, t_line.orig.y);
        cairo_line_to(cr, t_line.dest.x, t_line.dest.y);

        /*if (!xd->appending)
        {
            cairo_stroke(xd->cc);
            if (xd->currentMask >= 0)
            {
                /* If masking, use temporary pattern as source and mask that *s/
                cairo_pattern_t *source = cairo_pop_group(xd->cc);
                cairo_pattern_t *mask = xd->masks[xd->currentMask];
                cairo_set_source(xd->cc, source);
                cairo_mask(xd->cc, mask);
                /* Release temporary pattern *s/
                cairo_pattern_destroy(source);
            }
        }
        */
        cairo_stroke(cr);
    }

    void RendererCairo::polyline(const Polyline &t_polyline)
    {
        if (color::alpha(t_polyline.line.col) == 0)
            return;

        set_color(cr, t_polyline.line.col);
        set_linetype(cr, t_polyline.line);

        for (auto it = t_polyline.points.begin(); it != t_polyline.points.end(); ++it)
        {
            if (it == t_polyline.points.begin())
            {
                cairo_move_to(cr, it->x, it->y);
            }
            else
            {
                cairo_line_to(cr, it->x, it->y);
            }
        }
        cairo_stroke(cr);
    }

    void RendererCairo::polygon(const Polygon &t_polygon)
    {
    }

    void RendererCairo::path(const Path &t_path)
    {
    }

    void RendererCairo::raster(const Raster &t_raster)
    {
    }

}