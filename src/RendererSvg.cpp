#include "RendererSvg.h"

#include <cmath>
#include <fmt/ostream.h>
#include <functional>

#include "Base64.h"
#include "HttpgdRng.h"
#include "HttpgdCompress.h"

namespace httpgd::dc
{
    
    static inline void write_xml_escaped(fmt::memory_buffer &os, const std::string &text)
    {
        for (const char &c : text)
        {
            switch (c)
            {
            case '&':
                fmt::format_to(os, "&amp;");
                break;
            case '<':
                fmt::format_to(os, "&lt;");
                break;
            case '>':
                fmt::format_to(os, "&gt;");
                break;
            case '"':
                fmt::format_to(os, "&quot;");
                break;
            case '\'':
                fmt::format_to(os, "&apos;");
                break;
            default:
                fmt::format_to(os, "{}", c);
            }
        }
    }

    static inline void css_fill_or_none(fmt::memory_buffer &os, color_t col)
    {
        int alpha = color::alpha(col);
        if (alpha == 0)
        {
            fmt::format_to(os, "fill: none;");
        }
        else
        {
            fmt::format_to(os, "fill: #{:02X}{:02X}{:02X};", color::red(col), color::green(col), color::blue(col));
            if (alpha != 255)
            {
                fmt::format_to(os, "fill-opacity: {:.2f};", alpha / 255.0);
            }
        }
    }

    static inline void css_fill_or_omit(fmt::memory_buffer &os, color_t col)
    {
        int alpha = color::alpha(col);
        if (alpha != 0)
        {
            fmt::format_to(os, "fill: #{:02X}{:02X}{:02X};", color::red(col), color::green(col), color::blue(col));
            if (alpha != 255)
            {
                fmt::format_to(os, "fill-opacity: {:.2f};", alpha / 255.0);
            }
        }
    }

    static inline double scale_lty(int lty, double lwd)
    {
        // Don't rescale if lwd < 1
        // https://github.com/wch/r-source/blob/master/src/library/grDevices/src/cairo/cairoFns.c#L134
        return ((lwd > 1) ? lwd : 1) * (lty & 15);
    }
    static inline void css_lineinfo(fmt::memory_buffer &os, const LineInfo &line)
    {

        // 1 lwd = 1/96", but units in rest of document are 1/72"
        fmt::format_to(os, "stroke-width: {:.2f};", line.lwd / 96.0 * 72);

        // Default is "stroke: #000000;" as declared in <style>
        if (line.col != color::rgba(0, 0, 0, 255))
        {
            int alpha = color::alpha(line.col);
            if (alpha == 0)
            {
                fmt::format_to(os, "stroke: none;");
            }
            else
            {
                fmt::format_to(os, "stroke: #{:02X}{:02X}{:02X};", color::red(line.col), color::green(line.col), color::blue(line.col));
                if (alpha != color::byte_mask)
                {
                    fmt::format_to(os, "stroke-opacity: {:.2f};", color::byte_frac(alpha));
                }
            }
        }
        
        // Set line pattern type
        int lty = line.lty;
        switch (lty)
        {
        case LineInfo::LTY::BLANK : // never called: blank lines never get to this point
        case LineInfo::LTY::SOLID: // default svg setting, so don't need to write out
            break;
        default:
            // For details
            // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
            fmt::format_to(os, " stroke-dasharray: ");
            // First number
            fmt::format_to(os, "{:.2f}", scale_lty(lty, line.lwd));
            lty = lty >> 4;
            // Remaining numbers
            for (int i = 1; i < 8 && lty & 15; i++)
            {
                fmt::format_to(os, ", {:.2f}", scale_lty(lty, line.lwd));
                lty = lty >> 4;
            }
            fmt::format_to(os, ";");
            break;
        }

        // Set line end shape
        switch (line.lend)
        {
        case LineInfo::GC_ROUND_CAP: // declared to be default in <style>
            break;
        case LineInfo::GC_BUTT_CAP:
            fmt::format_to(os, "stroke-linecap: butt;");
            break;
        case LineInfo::GC_SQUARE_CAP:
            fmt::format_to(os, "stroke-linecap: square;");
            break;
        default:
            break;
        }

        // Set line join shape
        switch (line.ljoin)
        {
        case LineInfo::GC_ROUND_JOIN: // declared to be default in <style>
            break;
        case LineInfo::GC_BEVEL_JOIN:
            fmt::format_to(os, "stroke-linejoin: bevel;");
            break;
        case LineInfo::GC_MITRE_JOIN:
            fmt::format_to(os, "stroke-linejoin: miter;");
            if (std::fabs(line.lmitre - 10.0) > 1e-3)
            { // 10 is declared to be the default in <style>
                fmt::format_to(os, "stroke-miterlimit: {:.2f};", line.lmitre);
            }
            break;
        default:
            break;
        }
    }

    RendererSVG::RendererSVG(boost::optional<std::string> t_extra_css)
        : os(), m_extra_css(t_extra_css)
    {
    }
    
    void RendererSVG::render(const Page &t_page, double t_scale) 
    {
        m_scale = t_scale;
        this->page(t_page);
    }
    
    std::string RendererSVG::get_string() const 
    {
        return fmt::to_string(os);
    }
    
    void RendererSVG::page(const Page &t_page) 
    {
        os.reserve((t_page.dcs.size() + t_page.cps.size()) * 128 + 512);
        fmt::format_to(os, R""(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" class="httpgd" )"");
        fmt::format_to(os,
                   R""(width="{:.2f}" height="{:.2f}" viewBox="0 0 {:.2f} {:.2f}")"",
                   t_page.size.x * m_scale, t_page.size.y * m_scale, t_page.size.x, t_page.size.y);
        fmt::format_to(os, ">\n<defs>\n"
              "  <style type='text/css'><![CDATA[\n"
              "    .httpgd line, .httpgd polyline, .httpgd polygon, .httpgd path, .httpgd rect, .httpgd circle {{\n"
              "      fill: none;\n"
              "      stroke: #000000;\n"
              "      stroke-linecap: round;\n"
              "      stroke-linejoin: round;\n"
              "      stroke-miterlimit: 10.00;\n"
              "    }}\n");
        if (m_extra_css)
        {
            fmt::format_to(os, "{}\n", *m_extra_css);
        }
        fmt::format_to(os, 
              "  ]]></style>\n");

        for (const auto &cp : t_page.cps)
        {
            fmt::format_to(os, R""(<clipPath id="c{:d}"><rect x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}"/></clipPath>)"" "\n",
                    cp.id,
                    cp.rect.x,
                    cp.rect.y,
                    cp.rect.width,
                    cp.rect.height);
        }
        fmt::format_to(os, "</defs>\n");
        fmt::format_to(os, R""(<rect width="100%" height="100%" style="stroke: none;fill: #{:02X}{:02X}{:02X};"/>)"" "\n",
                   color::red(t_page.fill), color::green(t_page.fill), color::blue(t_page.fill));

        clip_id_t last_id = t_page.cps.front().id;
        fmt::format_to(os, R""(<g clip-path="url(#c{:d})">)"" "\n", last_id);
        for (const auto &dc : t_page.dcs)
        {
            if (dc->clip_id != last_id)
            {
                fmt::format_to(os, R""(</g><g clip-path="url(#c{:d})">)"" "\n", dc->clip_id);
                last_id = dc->clip_id;
            }
            dc->render(this);
            fmt::format_to(os, "\n");
        }
        fmt::format_to(os, "</g>\n</svg>");
    }

    void RendererSVG::dc(const DrawCall &)
    {
        fmt::format_to(os, "<!-- unknown draw call -->");
    }

    void RendererSVG::text(const Text &t_text)
    {
        // If we specify the clip path inside <image>, the "transform" also
        // affects the clip path, so we need to specify clip path at an outer level
        // (according to svglite)
        fmt::format_to(os, "<g><text ");

        if (t_text.rot == 0.0)
        {
            fmt::format_to(os, R""(x="{:.2f}" y="{:.2f}" )"", t_text.pos.x, t_text.pos.y);
        }
        else
        {
            fmt::format_to(os, R""(transform="translate({:.2f},{:.2f}) rotate({:.2f})" )"", t_text.pos.x, t_text.pos.y, t_text.rot * -1.0);
        }

        if (t_text.hadj == 0.5)
        {
            fmt::format_to(os, R""(text-anchor="middle" )"");
        }
        else if (t_text.hadj == 1)
        {
            fmt::format_to(os, R""(text-anchor="end" )"");
        }

        fmt::format_to(os, "style=\"");
        fmt::format_to(os, "font-family: {};font-size: {:.2f}px;", t_text.text.font_family, t_text.text.fontsize);

        if (t_text.text.weight != 400)
        {
            if (t_text.text.weight == 700)
            {
                fmt::format_to(os, "font-weight: bold;");
            }
            else
            {
                fmt::format_to(os, "font-weight: {};", t_text.text.weight);
            }
        }
        if (t_text.text.italic)
        {
            fmt::format_to(os, "font-style: italic;");
        }
        if (t_text.col != (int)color::rgb(0, 0, 0))
        {
            css_fill_or_none(os, t_text.col);
        }
        if (t_text.text.features.length() > 0)
        {
            fmt::format_to(os, "font-feature-settings: {};", t_text.text.features);
        }
        fmt::format_to(os, "\"");
        if (t_text.text.txtwidth_px > 0)
        {
            fmt::format_to(os, R""( textLength="{:.2f}px" lengthAdjust="spacingAndGlyphs")"", t_text.text.txtwidth_px);
        }
        fmt::format_to(os, ">");
        write_xml_escaped(os, t_text.str);
        fmt::format_to(os, "</text></g>");
    }

    void RendererSVG::circle(const Circle &t_circle)
    {
        fmt::format_to(os, "<circle ");
        fmt::format_to(os, R""(cx="{:.2f}" cy="{:.2f}" r="{:.2f}" )"", t_circle.pos.x, t_circle.pos.y, t_circle.radius);

        fmt::format_to(os, "style=\"");
        css_lineinfo(os, t_circle.line);
        css_fill_or_omit(os, t_circle.fill);
        fmt::format_to(os, "\"/>");
    }

    void RendererSVG::line(const Line &t_line)
    {
        fmt::format_to(os, "<line ");
        fmt::format_to(os, R""(x1="{:.2f}" y1="{:.2f}" x2="{:.2f}" y2="{:.2f}" )"", t_line.orig.x, t_line.orig.y, t_line.dest.x, t_line.dest.y);

        fmt::format_to(os, "style=\"");
        css_lineinfo(os, t_line.line);
        fmt::format_to(os, "\"/>");
    }

    void RendererSVG::rect(const Rect &t_rect)
    {
        fmt::format_to(os, "<rect ");
        fmt::format_to(os, R""(x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}" )"",
                   t_rect.rect.x,
                   t_rect.rect.y,
                   t_rect.rect.width,
                   t_rect.rect.height);

        fmt::format_to(os, "style=\"");
        css_lineinfo(os, t_rect.line);
        css_fill_or_omit(os, t_rect.fill);
        fmt::format_to(os, "\"/>");
    }

    void RendererSVG::polyline(const Polyline &t_polyline)
    {
        fmt::format_to(os, "<polyline points=\"");
        for (auto it = t_polyline.points.begin(); it != t_polyline.points.end(); ++it)
        {
            if (it != t_polyline.points.begin())
            {
                fmt::format_to(os, " ");
            }
            fmt::format_to(os, "{:.2f},{:.2f}", it->x, it->y);
        }
        fmt::format_to(os, "\" style=\"");
        css_lineinfo(os, t_polyline.line);
        fmt::format_to(os, "\"/>");
    }

    void RendererSVG::polygon(const Polygon &t_polygon)
    {
        fmt::format_to(os, "<polygon points=\"");
        for (auto it = t_polygon.points.begin(); it != t_polygon.points.end(); ++it)
        {
            if (it != t_polygon.points.begin())
            {
                fmt::format_to(os, " ");
            }
            fmt::format_to(os, "{:.2f},{:.2f}", it->x, it->y);
        }
        fmt::format_to(os, "\" ");

        fmt::format_to(os, "style=\"");
        css_lineinfo(os, t_polygon.line);
        css_fill_or_omit(os, t_polygon.fill);
        fmt::format_to(os, "\" ");

        fmt::format_to(os, "/>");
    }

    void RendererSVG::path(const Path &t_path)
    {
        fmt::format_to(os, "<path d=\"");

        auto it_poly = t_path.nper.begin();
        std::size_t left = 0;
        for (auto it = t_path.points.begin(); it != t_path.points.end(); ++it)
        {
            if (left == 0)
            {
                left = (*it_poly) - 1;
                ++it_poly;
                fmt::format_to(os, "M{:.2f} {:.2f}", it->x, it->y);
            }
            else
            {
                --left;
                fmt::format_to(os, "L{:.2f} {:.2f}", it->x, it->y);

                if (left == 0)
                {
                    fmt::format_to(os, "Z");
                }
            }
        }

        // Finish path data
        fmt::format_to(os, "\" style=\"");
        css_lineinfo(os, t_path.line);
        css_fill_or_omit(os, t_path.fill);
        fmt::format_to(os, "fill-rule: ");
        fmt::format_to(os, t_path.winding ? "nonzero" : "evenodd");
        fmt::format_to(os, ";\"/>");
    }
    
    void RendererSVG::raster(const Raster &t_raster)
    {
        // If we specify the clip path inside <image>, the "transform" also
        // affects the clip path, so we need to specify clip path at an outer level
        // (according to svglite)
        fmt::format_to(os, "<g><image ");
        fmt::format_to(os, R""( x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}" )"",
                   t_raster.rect.x,
                   t_raster.rect.y,
                   t_raster.rect.width,
                   t_raster.rect.height);
        fmt::format_to(os, R""(preserveAspectRatio="none" )"");
        if (!t_raster.interpolate)
        {
            fmt::format_to(os, R""(image-rendering="pixelated" )"");
        }
        if (t_raster.rot != 0)
        {
            fmt::format_to(os, R""(transform="rotate({:.2f},{:.2f},{:.2f})" )"", -1.0 * t_raster.rot, t_raster.rect.x, t_raster.rect.y);
        }
        fmt::format_to(os, " xlink:href=\"data:image/png;base64,");
        fmt::format_to(os, raster_base64(t_raster));
        fmt::format_to(os, "\"/></g>");
    }

    // Portable SVG renderer
    
    static inline void att_fill_or_none(fmt::memory_buffer &os, color_t col)
    {
        int alpha = color::alpha(col);
        if (alpha == 0)
        {
            fmt::format_to(os, R""( fill="none")"");
        }
        else
        {
            fmt::format_to(os, R""( fill="#{:02X}{:02X}{:02X}")"", color::red(col), color::green(col), color::blue(col));
            if (alpha != color::byte_mask)
            {
                fmt::format_to(os, R""( fill-opacity="{:.2f}")"", color::byte_frac(alpha));
            }
        }
    }

    static inline void att_fill_or_omit(fmt::memory_buffer &os, color_t col)
    {
        int alpha = color::alpha(col);
        if (alpha != 0)
        {
            fmt::format_to(os, R""( fill="#{:02X}{:02X}{:02X}")"", color::red(col), color::green(col), color::blue(col));
            if (alpha != color::byte_mask)
            {
                fmt::format_to(os, R""( fill-opacity="{:.2f}")"", color::byte_frac(alpha));
            }
        }
    }

    static inline void att_lineinfo(fmt::memory_buffer &os, const LineInfo &line)
    {

        // 1 lwd = 1/96", but units in rest of document are 1/72"
        fmt::format_to(os, R""(stroke-width="{:.2f}")"", line.lwd / 96.0 * 72);

        // Default is "stroke: none;"
        color_t alpha = color::alpha(line.col);
        if (alpha != 0)
        {
            fmt::format_to(os, R""( stroke="#{:02X}{:02X}{:02X}")"", color::red(line.col), color::green(line.col), color::blue(line.col));
            if (alpha != color::byte_mask)
            {
                fmt::format_to(os, R""( stroke-opacity="{:.2f}")"", color::byte_frac(alpha));
            }
        }
        
        // Set line pattern type
        int lty = line.lty;
        switch (lty)
        {
        case LineInfo::LTY::BLANK : // never called: blank lines never get to this point
        case LineInfo::LTY::SOLID: // default svg setting, so don't need to write out
            break;
        default:
            // For details
            // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
            fmt::format_to(os, R""( stroke-dasharray="{:.2f})"", scale_lty(lty, line.lwd));
            lty = lty >> 4;
            // Remaining numbers
            for (int i = 1; i < 8 && lty & 15; i++)
            {
                fmt::format_to(os, ", {:.2f}", scale_lty(lty, line.lwd));
                lty = lty >> 4;
            }
            fmt::format_to(os, "\"");
            break;
        }

        // Set line end shape
        switch (line.lend)
        {
        case LineInfo::GC_ROUND_CAP:
            fmt::format_to(os, R""( stroke-linecap="round")"");
            break;
        case LineInfo::GC_BUTT_CAP:
            // SVG default
            break;
        case LineInfo::GC_SQUARE_CAP:
            fmt::format_to(os, R""( stroke-linecap="square")"");
            break;
        default:
            break;
        }

        // Set line join shape
        switch (line.ljoin)
        {
        case LineInfo::GC_ROUND_JOIN: 
            fmt::format_to(os, R""( stroke-linejoin="round")"");
            break;
        case LineInfo::GC_BEVEL_JOIN:
            fmt::format_to(os, R""( stroke-linejoin="bevel")"");
            break;
        case LineInfo::GC_MITRE_JOIN:
            // default
            if (std::fabs(line.lmitre - 4.0) > 1e-3)
            { // 4 is the SVG default
                fmt::format_to(os, R""( stroke-miterlimit="{:.2f}")"", line.lmitre);
            }
            break;
        default:
            break;
        }
    }

    
    RendererSVGPortable::RendererSVGPortable() 
        : os()
    {
    }
    
    void RendererSVGPortable::render(const Page &t_page, double t_scale) 
    {
        m_unique_id = httpgd::rng::uuid();
        m_scale = t_scale;
        this->page(t_page);
    }
    
    std::string RendererSVGPortable::get_string() const 
    {
        return fmt::to_string(os);
    }
    
    void RendererSVGPortable::page(const Page &t_page) 
    {
        os.reserve((t_page.dcs.size() + t_page.cps.size()) * 128 + 512);
        fmt::format_to(os, R""(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" class="httpgd" )"");
        fmt::format_to(os,
                   R""(width="{:.2f}" height="{:.2f}" viewBox="0 0 {:.2f} {:.2f}">)"" "\n<defs>\n",
                   t_page.size.x * m_scale, t_page.size.y * m_scale, t_page.size.x, t_page.size.y);

        for (const auto &cp : t_page.cps)
        {
            fmt::format_to(os, R""(<clipPath id="c{:d}-{}"><rect x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}"/></clipPath>)"" "\n",
                    cp.id,
                    m_unique_id,
                    cp.rect.x,
                    cp.rect.y,
                    cp.rect.width,
                    cp.rect.height);
        }
        fmt::format_to(os, "</defs>\n");
        fmt::format_to(os, R""(<rect width="100%" height="100%" stroke="none" fill="#{:02X}{:02X}{:02X}"/>)"" "\n",
                   color::red(t_page.fill), color::green(t_page.fill), color::blue(t_page.fill));

        clip_id_t last_id = t_page.cps.front().id;
        fmt::format_to(os, R""(<g clip-path="url(#c{:d}-{})">)"" "\n", last_id, m_unique_id);
        for (const auto &dc : t_page.dcs)
        {
            if (dc->clip_id != last_id)
            {
                fmt::format_to(os, R""(</g><g clip-path="url(#c{:d}-{})">)"" "\n", dc->clip_id, m_unique_id);
                last_id = dc->clip_id;
            }
            dc->render(this);
            fmt::format_to(os, "\n");
        }
        fmt::format_to(os, "</g>\n</svg>");
    }
    
    void RendererSVGPortable::dc(const DrawCall &t_dc) 
    {
        fmt::format_to(os, "<!-- unknown draw call -->");
    }
    
    void RendererSVGPortable::rect(const Rect &t_rect) 
    {
        fmt::format_to(os, "<rect ");
        fmt::format_to(os, R""(x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}" )"",
                   t_rect.rect.x,
                   t_rect.rect.y,
                   t_rect.rect.width,
                   t_rect.rect.height);

        att_lineinfo(os, t_rect.line);
        att_fill_or_none(os, t_rect.fill);
        fmt::format_to(os, "/>");
    }
    
    void RendererSVGPortable::text(const Text &t_text) 
    {
        // If we specify the clip path inside <image>, the "transform" also
        // affects the clip path, so we need to specify clip path at an outer level
        // (according to svglite)
        fmt::format_to(os, "<g><text ");

        if (t_text.rot == 0.0)
        {
            fmt::format_to(os, R""(x="{:.2f}" y="{:.2f}" )"", t_text.pos.x, t_text.pos.y);
        }
        else
        {
            fmt::format_to(os, R""(transform="translate({:.2f},{:.2f}) rotate({:.2f})" )"", t_text.pos.x, t_text.pos.y, t_text.rot * -1.0);
        }

        if (t_text.hadj == 0.5)
        {
            fmt::format_to(os, R""(text-anchor="middle" )"");
        }
        else if (t_text.hadj == 1)
        {
            fmt::format_to(os, R""(text-anchor="end" )"");
        }

        fmt::format_to(os, R""(font-family="{}" font-size="{:.2f}px")"", t_text.text.font_family, t_text.text.fontsize);

        if (t_text.text.weight != 400)
        {
            if (t_text.text.weight == 700)
            {
                fmt::format_to(os, R""( font-weight="bold")"");
            }
            else
            {
                fmt::format_to(os, R""( font-weight="{}")"", t_text.text.weight);
            }
        }
        if (t_text.text.italic)
        {
            fmt::format_to(os, R""( font-style="italic")"");
        }
        if (t_text.col != color::rgb(0, 0, 0))
        {
            att_fill_or_none(os, t_text.col);
        }
        if (t_text.text.features.length() > 0)
        {
            fmt::format_to(os, R""( font-feature-settings="{}")"", t_text.text.features);
        }
        if (t_text.text.txtwidth_px > 0)
        {
            fmt::format_to(os, R""( textLength="{:.2f}px" lengthAdjust="spacingAndGlyphs")"", t_text.text.txtwidth_px);
        }
        fmt::format_to(os, ">");
        write_xml_escaped(os, t_text.str);
        fmt::format_to(os, "</text></g>");
    }
    
    void RendererSVGPortable::circle(const Circle &t_circle)
    {
        fmt::format_to(os, "<circle ");
        fmt::format_to(os, R""(cx="{:.2f}" cy="{:.2f}" r="{:.2f}" )"", t_circle.pos.x, t_circle.pos.y, t_circle.radius);

        att_lineinfo(os, t_circle.line);
        att_fill_or_none(os, t_circle.fill);
        fmt::format_to(os, "/>");
    }

    void RendererSVGPortable::line(const Line &t_line)
    {
        fmt::format_to(os, "<line ");
        fmt::format_to(os, R""(x1="{:.2f}" y1="{:.2f}" x2="{:.2f}" y2="{:.2f}" )"", t_line.orig.x, t_line.orig.y, t_line.dest.x, t_line.dest.y);

        att_lineinfo(os, t_line.line);
        fmt::format_to(os, "/>");
    }
    
    void RendererSVGPortable::polyline(const Polyline &t_polyline) 
    {
        fmt::format_to(os, "<polyline points=\"");
        for (auto it = t_polyline.points.begin(); it != t_polyline.points.end(); ++it)
        {
            if (it != t_polyline.points.begin())
            {
                fmt::format_to(os, " ");
            }
            fmt::format_to(os, "{:.2f},{:.2f}", it->x, it->y);
        }
        fmt::format_to(os, "\" fill=\"none\" ");
        att_lineinfo(os, t_polyline.line);
        fmt::format_to(os, "/>");
    }
    
    void RendererSVGPortable::polygon(const Polygon &t_polygon) 
    {
        fmt::format_to(os, "<polygon points=\"");
        for (auto it = t_polygon.points.begin(); it != t_polygon.points.end(); ++it)
        {
            if (it != t_polygon.points.begin())
            {
                fmt::format_to(os, " ");
            }
            fmt::format_to(os, "{:.2f},{:.2f}", it->x, it->y);
        }
        fmt::format_to(os, "\" ");
        att_lineinfo(os, t_polygon.line);
        att_fill_or_none(os, t_polygon.fill);
        fmt::format_to(os, "/>");
    }
    
    void RendererSVGPortable::path(const Path &t_path) 
    {
        fmt::format_to(os, "<path d=\"");

        auto it_poly = t_path.nper.begin();
        std::size_t left = 0;
        for (auto it = t_path.points.begin(); it != t_path.points.end(); ++it)
        {
            if (left == 0)
            {
                left = (*it_poly) - 1;
                ++it_poly;
                fmt::format_to(os, "M{:.2f} {:.2f}", it->x, it->y);
            }
            else
            {
                --left;
                fmt::format_to(os, "L{:.2f} {:.2f}", it->x, it->y);

                if (left == 0)
                {
                    fmt::format_to(os, "Z");
                }
            }
        }

        // Finish path data
        fmt::format_to(os, "\" ");
        att_lineinfo(os, t_path.line);
        att_fill_or_none(os, t_path.fill);
        fmt::format_to(os, " fill-rule=\"");
        fmt::format_to(os, t_path.winding ? "nonzero" : "evenodd");
        fmt::format_to(os, "\"/>");
    }
    
    void RendererSVGPortable::raster(const Raster &t_raster) 
    {
        // If we specify the clip path inside <image>, the "transform" also
        // affects the clip path, so we need to specify clip path at an outer level
        // (according to svglite)
        fmt::format_to(os, "<g><image ");
        fmt::format_to(os, R""( x="{:.2f}" y="{:.2f}" width="{:.2f}" height="{:.2f}" )"",
                   t_raster.rect.x,
                   t_raster.rect.y,
                   t_raster.rect.width,
                   t_raster.rect.height);
        fmt::format_to(os, R""(preserveAspectRatio="none" )"");
        if (!t_raster.interpolate)
        {
            fmt::format_to(os, R""(image-rendering="pixelated" )"");
        }
        if (t_raster.rot != 0)
        {
            fmt::format_to(os, R""(transform="rotate({:.2f},{:.2f},{:.2f})" )"", -1.0 * t_raster.rot, t_raster.rect.x, t_raster.rect.y);
        }
        fmt::format_to(os, " xlink:href=\"data:image/png;base64,");
        fmt::format_to(os, raster_base64(t_raster));
        fmt::format_to(os, "\"/></g>");
    }

    RendererSVGZ::RendererSVGZ(boost::optional<std::string> t_extra_css) :
        RendererSVG(t_extra_css)
    {
    }
    
    std::vector<unsigned char> RendererSVGZ::get_binary() const 
    {
        return compr::compress_str(RendererSVG::get_string());
    }
    
    RendererSVGZPortable::RendererSVGZPortable() :
        RendererSVGPortable()
    {
    }
    
    std::vector<unsigned char> RendererSVGZPortable::get_binary() const 
    {
        return compr::compress_str(RendererSVGPortable::get_string());
    }

} // namespace httpgd::dc
