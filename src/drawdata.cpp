#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <gdtools.h>

#include <R_ext/GraphicsDevice.h>
#include <R_ext/GraphicsEngine.h>

#include "DrawData.h"

namespace httpgd
{
    namespace dc
    {

        template <typename T>
        inline void svg_field(std::string *buf, const std::string &name, T val)
        {
            buf->append(name).append("=\"").append(std::to_string(val)).append("\" ");
        }
        template <>
        inline void svg_field<const char *>(std::string *buf, const std::string &name, const char *val)
        {
            buf->append(name).append("=\"").append(val).append("\" ");
        }
        template <>
        inline void svg_field<std::string>(std::string *buf, const std::string &name, std::string val)
        {
            buf->append(name).append("=\"").append(val).append("\" ");
        }

        inline void svg_elem(std::string *buf, const std::string &name)
        {
            buf->append("<").append(name).append(" ");
        }

        template <typename T>
        inline void css_field(std::string *buf, const std::string &name, T val)
        {
            buf->append(name).append(": ").append(std::to_string(val)).append(";");
        }
        template <>
        inline void css_field<const char *>(std::string *buf, const std::string &name, const char *val)
        {
            buf->append(name).append(": ").append(val).append(";");
        }

        inline void css_color(std::string *buf, int c)
        {
            char hexcol[17];
            snprintf(hexcol, sizeof hexcol, "#%02X%02X%02X", R_RED(c), R_GREEN(c), R_BLUE(c));
            (*buf) += hexcol;
        }
        inline void css_field_color(std::string *buf, const std::string &name, int c)
        {
            buf->append(name).append(": ");
            css_color(buf, c);
            buf->append(";");
        }

        inline void write_style_col(std::string *buf, const DrawCall *const dc)
        {
            buf->append("fill: ");
            int col = dc->m_fill;
            int alpha = R_ALPHA(col);
            if (alpha == 0)
            {
                buf->append(": none;");
            }
            else
            {
                css_color(buf, col);
                buf->append(";");
                if (alpha != 255)
                {
                    buf->append(" fill-opacity: ").append(std::to_string(alpha / 255.0)).append(";");
                }
            }
        }

        inline double scale_lty(int lty, double lwd)
        {
            // Don't rescale if lwd < 1
            // https://github.com/wch/r-source/blob/master/src/library/grDevices/src/cairo/cairoFns.c#L134
            return ((lwd > 1) ? lwd : 1) * (lty & 15);
        }
        inline void write_style_linetype(std::string *buf, const DrawCall *const dc)
        {
            int lty = dc->m_lty;

            // 1 lwd = 1/96", but units in rest of document are 1/72"
            css_field(buf, "stroke-width", dc->m_lwd / 96.0 * 72);

            // Default is "stroke: #000000;" as declared in <style>
            if (dc->m_col != R_RGBA(0, 0, 0, 255)) {
                css_field_color(buf, "stroke", dc->m_col);
            }

            // Set line pattern type
            switch (lty)
            {
            case LTY_BLANK: // never called: blank lines never get to this point
                buf->append("<!--BLANK-->");
            case LTY_SOLID: // default svg setting, so don't need to write out
                break;
            default:
                // For details
                // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
                buf->append(" stroke-dasharray: ");
                // First number
                buf->append(std::to_string(scale_lty(lty, dc->m_lwd)));
                lty = lty >> 4;
                // Remaining numbers
                for (int i = 1; i < 8 && lty & 15; i++)
                {
                    buf->append(",").append(std::to_string(scale_lty(lty, dc->m_lwd)));
                    lty = lty >> 4;
                }
                buf->append(";");
                break;
            }

            // Set line end shape
            switch (dc->m_lend)
            {
            case GC_ROUND_CAP: // declared to be default in <style>
                break;
            case GC_BUTT_CAP:
                css_field(buf, "stroke-linecap", "butt");
                break;
            case GC_SQUARE_CAP:
                css_field(buf, "stroke-linecap", "square");
                break;
            default:
                break;
            }

            // Set line join shape
            switch (dc->m_ljoin)
            {
            case GC_ROUND_JOIN: // declared to be default in <style>
                break;
            case GC_BEVEL_JOIN:
                css_field(buf, "stroke-linejoin", "bevel");
                break;
            case GC_MITRE_JOIN:
                css_field(buf, "stroke-linejoin", "miter");
                if (std::abs(dc->m_lmitre - 10.0) > 1e-3) { // 10 is declared to be the default in <style>
                    css_field(buf, "stroke-miterlimit", dc->m_lmitre);
                }
                break;
            default:
                break;
            }
        }

        // DRAW CALL OBJECTS

        DrawCall::DrawCall(const void *gcv)
        {
            auto gc = (const pGEcontext)(gcv);
            m_col = gc->col;
            m_fill = gc->fill;
            m_gamma = gc->gamma;
            m_lwd = gc->lwd;
            m_lty = gc->lty;
            m_lend = static_cast<dc::GC_lineend>(gc->lend);
            m_ljoin = static_cast<dc::GC_linejoin>(gc->ljoin);
            m_lmitre = gc->lmitre;
        }
        DrawCall::~DrawCall() = default;
        void DrawCall::build_svg(std::string *buf) const
        {
            buf->append("<!-- unknown draw call -->");
        }

        void DrawCall::build_svg_style(std::string *buf, bool include_fill) const
        {
            buf->append("style=\"");
            write_style_linetype(buf, this);
            if (include_fill && R_ALPHA(m_fill) != 0)
            {
                write_style_col(buf, this);
            }
            buf->append("\" ");
        }

        Text::Text(const void *gc, double x, double y, const std::string &str, double rot, double hadj, const TextInfo &t_text)
            : DrawCall(gc),
              m_x(x), m_y(y), m_rot(rot), m_hadj(hadj), m_str(str), m_text(t_text)
        {
        }
        void Text::build_svg(std::string *buf) const
        {
            // If we specify the clip path inside <image>, the "transform" also
            // affects the clip path, so we need to specify clip path at an outer level
            // (according to svglite)
            svg_elem(buf, "g");
            m_clip->build_svg_attr(buf);
            buf->append(">");

            svg_elem(buf, "text");
            if (m_rot == 0.0)
            {
                svg_field(buf, "x", m_x);
                svg_field(buf, "y", m_y);
            }
            else
            {
                buf->append("transform=\"translate(");
                buf->append(std::to_string(m_x));
                buf->append(",");
                buf->append(std::to_string(m_y));
                buf->append(") rotate(");
                buf->append(std::to_string(m_rot * -1.0));
                buf->append(")\" ");
            }
            svg_field(buf, "font-family", m_text.font_family);
            svg_field(buf, "font-size", m_text.fontsize);
            if (m_text.bold)
            {
                svg_field(buf, "font-weight", "bold");
            }
            if (m_text.italic)
            {
                svg_field(buf, "font-style", "italic");
            }
            // todo: libsvg also sets the text width in pixels here

            buf->append(">").append(m_str).append("</text></g>");
        }

        Circle::Circle(const void *gc,
                       double x, double y, double r)
            : DrawCall(gc),
              m_x(x), m_y(y), m_r(r)
        {
        }
        void Circle::build_svg(std::string *buf) const
        {
            svg_elem(buf, "circle");
            m_clip->build_svg_attr(buf);
            svg_field(buf, "cx", m_x);
            svg_field(buf, "cy", m_y);
            svg_field(buf, "r", m_r);

            buf->append("style=\"");
            write_style_linetype(buf, this);
            if (R_ALPHA(m_fill) != 0)
            {
                write_style_col(buf, this);
            }
            buf->append("\" ");

            buf->append("/>");
        }

        Line::Line(const void *gc,
                   double x1, double y1, double x2, double y2)
            : DrawCall(gc),
              m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
        {
        }
        void Line::build_svg(std::string *buf) const
        {
            svg_elem(buf, "line");
            m_clip->build_svg_attr(buf);
            svg_field(buf, "x1", m_x1);
            svg_field(buf, "y1", m_y1);
            svg_field(buf, "x2", m_x2);
            svg_field(buf, "y2", m_y2);

            build_svg_style(buf, false);

            buf->append("/>");
        }

        Rect::Rect(const void *gc,
                   double x0, double y0, double x1, double y1)
            : DrawCall(gc),
              m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1)
        {
        }
        void Rect::build_svg(std::string *buf) const
        {
            svg_elem(buf, "rect");
            m_clip->build_svg_attr(buf);
            svg_field(buf, "x", std::min(m_x0, m_x1));
            svg_field(buf, "y", std::min(m_y0, m_y1));
            svg_field(buf, "width", std::abs(m_x0 - m_x1));
            svg_field(buf, "height", std::abs(m_y0 - m_y1));

            build_svg_style(buf, true);

            buf->append("/>");
        }

        Polyline::Polyline(const void *gc,
                           int n, const std::vector<double> &x, const std::vector<double> &y)
            : DrawCall(gc),
              m_n(n), m_x(x), m_y(y)
        {
        }
        void Polyline::build_svg(std::string *buf) const
        {
            svg_elem(buf, "polyline");
            m_clip->build_svg_attr(buf);
            std::string pts = "";
            for (int i = 0; i < m_n; i++)
            {
                pts += std::to_string(m_x[i]).append(",");
                pts += std::to_string(m_y[i]).append(" ");
            }
            svg_field(buf, "points", pts);

            build_svg_style(buf, false);

            buf->append("/>");
        }
        Polygon::Polygon(const void *gc,
                         int n, const std::vector<double> &x, const std::vector<double> &y)
            : DrawCall(gc),
              m_n(n), m_x(x), m_y(y)
        {
        }
        void Polygon::build_svg(std::string *buf) const
        {
            svg_elem(buf, "polygon");
            m_clip->build_svg_attr(buf);
            std::string pts = "";
            for (int i = 0; i < m_n; i++)
            {
                pts += std::to_string(m_x[i]).append(",");
                pts += std::to_string(m_y[i]).append(" ");
            }
            svg_field(buf, "points", pts);

            build_svg_style(buf, true);

            buf->append("/>");
        }
        Path::Path(const void *gc,
                   const std::vector<double> &x, const std::vector<double> &y, int npoly, const std::vector<int> &nper, bool winding)
            : DrawCall(gc),
              m_x(x), m_y(y), m_npoly(npoly), m_nper(nper), m_winding(winding)
        {
        }
        void Path::build_svg(std::string *buf) const
        {
            svg_elem(buf, "path");
            m_clip->build_svg_attr(buf);
            // Create path data
            buf->append("d=\"");
            int ind = 0;
            for (int i = 0; i < m_npoly; i++)
            {
                // Move to the first point of the sub-path
                buf->append("M ");
                buf->append(std::to_string(m_x[ind]));
                buf->append(" ");
                buf->append(std::to_string(m_y[ind]));
                buf->append(" ");
                ind++;
                // Draw the sub-path
                for (int j = 1; j < m_nper[i]; j++)
                {
                    buf->append("L ");
                    buf->append(std::to_string(m_x[ind]));
                    buf->append(" ");
                    buf->append(std::to_string(m_y[ind]));
                    buf->append(" ");
                    ind++;
                }
                // Close the sub-path
                buf->append("Z");
            }
            // Finish path data
            buf->append("\" ");

            buf->append("style\"");
            write_style_linetype(buf, this);
            if (R_ALPHA(m_fill) != 0)
            {
                write_style_col(buf, this);
            }
            buf->append("fill-rule: ");
            buf->append(m_winding ? "nonzero" : "evenodd");
            buf->append(";");
            buf->append("\" ");

            buf->append("/>");
        }

        Raster::Raster(const void *gc,
                       const std::vector<unsigned int> &raster, int w, int h,
                       double x, double y,
                       double width, double height,
                       double rot,
                       bool interpolate)
            : DrawCall(gc),
              m_raster(raster), m_w(w), m_h(h), m_x(x), m_y(y), m_width(width), m_height(height), m_rot(rot), m_interpolate(interpolate)
        {
        }
        void Raster::build_svg(std::string *buf) const
        {
            double imageHeight = m_height;
            double imageWidth = m_width;

            if (m_height < 0)
            {
                imageHeight = -m_height;
            }
            if (m_width < 0)
            {
                imageWidth = -m_width;
            }

            // If we specify the clip path inside <image>, the "transform" also
            // affects the clip path, so we need to specify clip path at an outer level
            // (according to svglite)
            svg_elem(buf, "g");
            m_clip->build_svg_attr(buf);
            buf->append(">");

            svg_elem(buf, "image");
            svg_field(buf, "width", imageWidth);
            svg_field(buf, "height", imageHeight);
            svg_field(buf, "x", m_x);
            svg_field(buf, "y", m_y - imageHeight);
            if (m_rot != 0)
            {
                buf->append("transform=\"rotate(");
                buf->append(std::to_string(-1.0 * m_rot));
                buf->append(",");
                buf->append(std::to_string(m_x));
                buf->append(",");
                buf->append(std::to_string(m_y));
                buf->append(")\" ");
            }
            buf->append(" xlink:href=\"data:image/png;base64,");
            buf->append(gdtools::raster_to_str(m_raster, m_w, m_h, imageWidth, imageHeight, m_interpolate));
            buf->append("\"/></g>");
        }

        Clip::Clip(int id, double x0, double x1, double y0, double y1)
            : m_id(id), m_x0(x0), m_x1(x1), m_y0(y0), m_y1(y1)
        {
        }
        const double CLIP_EPSILON = 0.01;
        bool Clip::equals(double x0, double x1, double y0, double y1)
        {
            return std::abs(x0 - m_x0) < CLIP_EPSILON &&
                   std::abs(x1 - m_x1) < CLIP_EPSILON &&
                   std::abs(y0 - m_y0) < CLIP_EPSILON &&
                   std::abs(y1 - m_y1) < CLIP_EPSILON;
        }
        void Clip::build_svg_def(std::string *buf) const
        {
            buf->append("<clipPath id=\"cp").append(std::to_string(m_id)).append("\">");
            buf->append("<rect x=\"").append(std::to_string(std::min(m_x0, m_x1)));
            buf->append("\" y=\"").append(std::to_string(std::min(m_y0, m_y1)));
            buf->append("\" width=\"").append(std::to_string(std::abs(m_x1 - m_x0)));
            buf->append("\" height=\"").append(std::to_string(std::abs(m_y1 - m_y0)));
            buf->append("\" /></clipPath>");
        }
        void Clip::build_svg_attr(std::string *buf) const
        {
            buf->append("clip-path=\"url(#cp").append(std::to_string(m_id)).append(")\" ");
        }

        Page::Page(double t_width, double t_height)
            : width(t_width), height(t_height), m_dcs(), m_cps(), m_upid(0)
        {
            clip(0, width, 0, height);
        }
        Page::~Page()
        {
            for (auto p : m_dcs)
            {
                delete p;
            } 
            m_dcs.clear();
        }

        void Page::clip(double x0, double x1, double y0, double y1)
        {
            if (m_cps.size() == 0 || !m_cps.back().equals(x0, x1, y0, y1))
            {
                m_cps.emplace_back(Clip(m_cps.size(), x0, x1, y0, y1));
            }
        }

        void Page::put(DrawCall *dc)
        {
            m_dcs.emplace_back(dc);
            dc->m_clip = &m_cps.back();
            m_upid++;
        }

        void Page::clear()
        {
            for (auto p : m_dcs)
            {
                delete p;
            } 
            m_dcs.clear();
            m_cps.clear();
            clip(0, width, 0, height);
            m_upid++;
        }
        int Page::get_upid()
        {
            return m_upid;
        }
        void Page::build_svg(std::string *buf) const
        {
            svg_elem(buf, "svg");
            svg_field(buf, "xmlns", "http://www.w3.org/2000/svg");
            svg_field(buf, "xmlns:xlink", "http://www.w3.org/1999/xlink");
            svg_field(buf, "width", std::to_string(width));
            svg_field(buf, "height", std::to_string(height));
            buf->append("viewBox=\"0 0 ");
            buf->append(std::to_string(width)).append(" ").append(std::to_string(height));
            buf->append("\"");

            (*buf) += "><defs>\n"
                      "  <style type='text/css'><![CDATA[\n"
                      "    line, polyline, polygon, path, rect, circle {\n"
                      "      fill: none;\n"
                      "      stroke: #000000;\n"
                      "      stroke-linecap: round;\n"
                      "      stroke-linejoin: round;\n"
                      "      stroke-miterlimit: 10.00;\n"
                      "    }\n"
                      "  ]]></style>\n";

            std::for_each(m_cps.begin(), m_cps.end(), [&](const Clip &asd) { buf->append("  "); asd.build_svg_def(buf); buf->append("\n"); });

            buf->append("</defs>\n");

            (*buf) += "<rect width='100%' height='100%' "
                      "style=\"stroke: none; ";
            css_field_color(buf, "fill", fill);
            buf->append("\"/>\n");

            std::for_each(m_dcs.begin(), m_dcs.end(), [&](const DrawCall *const piece) { buf->append("  "); piece->build_svg(buf); buf->append("\n"); });
            buf->append("</svg>");
        }

    } // namespace dc
} // namespace httpgd