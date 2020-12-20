
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fmt/ostream.h>

//#include <Rcpp.h>
//#include <R_ext/GraphicsEngine.h>

#include "lib/svglite_utils.h"

#include "DrawData.h"

namespace httpgd
{
    namespace dc
    {

        inline void svg_field(std::ostream &os, const std::string &name, const double val)
        {
            fmt::print(os, "{}=\"{:.2f}\" ", name, val);
        }
        inline void svg_field(std::ostream &os, const std::string &name, const char *val)
        {
            os << name << "=\"" << val << "\" ";
        }
        inline void svg_field(std::ostream &os, const std::string &name, const std::string &val)
        {
            os << name << "=\"" << val << "\" ";
        }

        inline void svg_elem(std::ostream &os, const std::string &name)
        {
            os << "<" << name << " ";
        }

        inline void css_field(std::ostream &os, const std::string &name, double val)
        {
            fmt::print(os, "{}: {:.2f};", name, val);
        }
        inline void css_field(std::ostream &os, const std::string &name, const char *val)
        {
            os << name << ": " << val << ";";
        }

        inline void css_color(std::ostream &os, int c)
        {
            fmt::print(os, "#{:02X}{:02X}{:02X}", R_RED(c), R_GREEN(c), R_BLUE(c));
        }
        inline void css_field_color(std::ostream &os, const std::string &name, int c)
        {
            fmt::print(os, "{}: #{:02X}{:02X}{:02X};", name, R_RED(c), R_GREEN(c), R_BLUE(c));
        }

        inline void write_style_col(std::ostream &os, const int col)
        {
            os << "fill: ";
            int alpha = R_ALPHA(col);
            if (alpha == 0)
            {
                os << ": none;";
            }
            else
            {
                css_color(os, col);
                os << ";";
                if (alpha != 255)
                {
                    fmt::print(os, " fill-opacity: {:.2f};", alpha / 255.0);
                }
            }
        }

        inline double scale_lty(int lty, double lwd)
        {
            // Don't rescale if lwd < 1
            // https://github.com/wch/r-source/blob/master/src/library/grDevices/src/cairo/cairoFns.c#L134
            return ((lwd > 1) ? lwd : 1) * (lty & 15);
        }
        inline void write_style_linetype(std::ostream &os, const DrawCall *const dc)
        {
            int lty = dc->m_lty;

            // 1 lwd = 1/96", but units in rest of document are 1/72"
            css_field(os, "stroke-width", dc->m_lwd / 96.0 * 72);

            // Default is "stroke: #000000;" as declared in <style>
            if (dc->m_col != R_RGBA(0, 0, 0, 255))
            {
                int alpha = R_ALPHA(dc->m_col);
                if (alpha == 0)
                {
                    os << "stroke: none;";
                }
                else
                {
                    css_field_color(os, "stroke", dc->m_col);
                    if (alpha != 255)
                    {
                        fmt::print(os, " stroke-opacity: {:.2f};", alpha / 255.0);
                    }
                }
            }

            // Set line pattern type
            switch (lty)
            {
            case LTY_BLANK: // never called: blank lines never get to this point
                os << "<!--BLANK-->";
            case LTY_SOLID: // default svg setting, so don't need to write out
                break;
            default:
                // For details
                // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
                os << " stroke-dasharray: ";
                // First number
                fmt::print(os, "{:.2f}", scale_lty(lty, dc->m_lwd));
                lty = lty >> 4;
                // Remaining numbers
                for (int i = 1; i < 8 && lty & 15; i++)
                {
                    fmt::print(os, ", {:.2f}", scale_lty(lty, dc->m_lwd));
                    lty = lty >> 4;
                }
                os << ";";
                break;
            }

            // Set line end shape
            switch (dc->m_lend)
            {
            case GC_ROUND_CAP: // declared to be default in <style>
                break;
            case GC_BUTT_CAP:
                css_field(os, "stroke-linecap", "butt");
                break;
            case GC_SQUARE_CAP:
                css_field(os, "stroke-linecap", "square");
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
                css_field(os, "stroke-linejoin", "bevel");
                break;
            case GC_MITRE_JOIN:
                css_field(os, "stroke-linejoin", "miter");
                if (std::abs(dc->m_lmitre - 10.0) > 1e-3)
                { // 10 is declared to be the default in <style>
                    css_field(os, "stroke-miterlimit", dc->m_lmitre);
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
        void DrawCall::build_svg(std::ostream &os) const
        {
            os << "<!-- unknown draw call -->";
        }

        void DrawCall::build_svg_style(std::ostream &os, bool include_fill) const
        {
            os << "style=\"";
            write_style_linetype(os, this);
            if (include_fill && R_ALPHA(m_fill) != 0)
            {
                write_style_col(os, this->m_fill);
            }
            os << "\" ";
        }

        Text::Text(const void *gc, double x, double y, const std::string &str, double rot, double /*hadj*/, const TextInfo &t_text)
            : DrawCall(gc),
              m_x(x), m_y(y), m_rot(rot), /*m_hadj(hadj),*/ m_str(str), m_text(t_text)
        {
        }
        void Text::build_svg(std::ostream &os) const
        {
            // If we specify the clip path inside <image>, the "transform" also
            // affects the clip path, so we need to specify clip path at an outer level
            // (according to svglite)
            svg_elem(os, "g");
            Clip::build_svg_attr(os, m_clip_id);
            os << ">";

            svg_elem(os, "text");
            if (m_rot == 0.0)
            {
                svg_field(os, "x", m_x);
                svg_field(os, "y", m_y);
            }
            else
            {
                fmt::print(os, "transform=\"translate({:.2f},{:.2f}) rotate({:.2f})\" ", m_x, m_y, m_rot * -1.0);
            }
            svg_field(os, "font-family", m_text.font_family);
            svg_field(os, "font-size", m_text.fontsize);
            if (m_text.bold)
            {
                svg_field(os, "font-weight", "bold");
            }
            if (m_text.italic)
            {
                svg_field(os, "font-style", "italic");
            }
            if (m_col != (int)R_RGB(0, 0, 0))
            {
                os << "style=\"";
                write_style_col(os, this->m_col);
                os << "\" ";
            }
            // todo: libsvg also sets the text width in pixels here

            os << ">";
            write_xml_escaped(os, m_str);
            os << "</text></g>";
        }

        Circle::Circle(const void *gc,
                       double x, double y, double r)
            : DrawCall(gc),
              m_x(x), m_y(y), m_r(r)
        {
        }
        void Circle::build_svg(std::ostream &os) const
        {
            svg_elem(os, "circle");
            Clip::build_svg_attr(os, m_clip_id);
            svg_field(os, "cx", m_x);
            svg_field(os, "cy", m_y);
            svg_field(os, "r", m_r);

            os << "style=\"";
            write_style_linetype(os, this);
            if (R_ALPHA(m_fill) != 0)
            {
                write_style_col(os, this->m_fill);
            }
            os << "\" ";

            os << "/>";
        }

        Line::Line(const void *gc,
                   double x1, double y1, double x2, double y2)
            : DrawCall(gc),
              m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
        {
        }
        void Line::build_svg(std::ostream &os) const
        {
            svg_elem(os, "line");
            Clip::build_svg_attr(os, m_clip_id);
            svg_field(os, "x1", m_x1);
            svg_field(os, "y1", m_y1);
            svg_field(os, "x2", m_x2);
            svg_field(os, "y2", m_y2);

            build_svg_style(os, false);

            os << "/>";
        }

        Rect::Rect(const void *gc,
                   double x0, double y0, double x1, double y1)
            : DrawCall(gc),
              m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1)
        {
        }
        void Rect::build_svg(std::ostream &os) const
        {
            svg_elem(os, "rect");
            Clip::build_svg_attr(os, m_clip_id);
            svg_field(os, "x", std::min(m_x0, m_x1));
            svg_field(os, "y", std::min(m_y0, m_y1));
            svg_field(os, "width", std::abs(m_x0 - m_x1));
            svg_field(os, "height", std::abs(m_y0 - m_y1));

            build_svg_style(os, true);

            os << "/>";
        }

        Polyline::Polyline(const void *gc,
                           int n, const std::vector<double> &x, const std::vector<double> &y)
            : DrawCall(gc),
              m_n(n), m_x(x), m_y(y)
        {
        }
        void Polyline::build_svg(std::ostream &os) const
        {
            svg_elem(os, "polyline");
            Clip::build_svg_attr(os, m_clip_id);

            os << "points=\"";
            for (int i = 0; i < m_n; ++i)
            {
                fmt::print(os, "{:.2f},{:.2f} ", m_x[i], m_y[i]);
            }
            os << "\" ";

            build_svg_style(os, false);

            os << "/>";
        }
        Polygon::Polygon(const void *gc,
                         int n, const std::vector<double> &x, const std::vector<double> &y)
            : DrawCall(gc),
              m_n(n), m_x(x), m_y(y)
        {
        }
        void Polygon::build_svg(std::ostream &os) const
        {
            svg_elem(os, "polygon");
            Clip::build_svg_attr(os, m_clip_id);

            os << "points=\"";
            for (int i = 0; i < m_n; ++i)
            {
                fmt::print(os, "{:.2f},{:.2f} ", m_x[i], m_y[i]);
            }
            os << "\" ";

            build_svg_style(os, true);

            os << "/>";
        }
        Path::Path(const void *gc,
                   const std::vector<double> &x, const std::vector<double> &y, int npoly, const std::vector<int> &nper, bool winding)
            : DrawCall(gc),
              m_x(x), m_y(y), m_npoly(npoly), m_nper(nper), m_winding(winding)
        {
        }
        void Path::build_svg(std::ostream &os) const
        {
            svg_elem(os, "path");
            Clip::build_svg_attr(os, m_clip_id);
            // Create path data
            os << "d=\"";
            int ind = 0;
            for (int i = 0; i < m_npoly; i++)
            {
                // Move to the first point of the sub-path
                fmt::print(os, "M {:.2f} {:.2f} ", m_x[ind], m_y[ind]);
                ind++;
                // Draw the sub-path
                for (int j = 1; j < m_nper[i]; j++)
                {
                    fmt::print(os, "L {:.2f} {:.2f} ", m_x[ind], m_y[ind]);
                    ind++;
                }
                // Close the sub-path
                os << "Z";
            }
            // Finish path data
            os << "\" "
               << "style=\"";
            write_style_linetype(os, this);
            if (R_ALPHA(m_fill) != 0)
            {
                write_style_col(os, this->m_fill);
            }
            os << "fill-rule: "
               << (m_winding ? "nonzero" : "evenodd")
               << ";"
               << "\" "
               << "/>";
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
        void Raster::build_svg(std::ostream &os) const
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
            svg_elem(os, "g");
            Clip::build_svg_attr(os, m_clip_id);
            os << ">";

            svg_elem(os, "image");
            svg_field(os, "width", imageWidth);
            svg_field(os, "height", imageHeight);
            svg_field(os, "x", m_x);
            svg_field(os, "y", m_y - imageHeight);
            svg_field(os, "preserveAspectRatio", "none");
            if (!m_interpolate)
            {
                svg_field(os, "image-rendering", "pixelated");
            }
            if (m_rot != 0)
            {
                fmt::print(os, "transform=\"rotate({:.2f},{:.2f},{:.2f})\" ", -1.0 * m_rot, m_x, m_y);
            }
            os << " xlink:href=\"data:image/png;base64,";
            os << raster_to_string(m_raster, m_w, m_h, imageWidth, imageHeight, m_interpolate);
            os << "\"/></g>";
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
        int Clip::id() const
        {
            return m_id;
        }
        void Clip::build_svg_def(std::ostream &os) const
        {
            os << "<clipPath id=\"c" << fmt::format("{:d}", m_id) << "\">"
               << "<rect x=\"" << fmt::format("{:.2f}", std::min(m_x0, m_x1))
               << "\" y=\"" << fmt::format("{:.2f}", std::min(m_y0, m_y1))
               << "\" width=\"" << fmt::format("{:.2f}", std::abs(m_x1 - m_x0))
               << "\" height=\"" << fmt::format("{:.2f}", std::abs(m_y1 - m_y0))
               << "\" /></clipPath>";
        }
        void Clip::build_svg_attr(std::ostream &os, int id)
        {
            fmt::print(os, "clip-path=\"url(#c{:d})\" ", id);
        }

        Page::Page(double t_width, double t_height)
            : width(t_width), height(t_height), m_dcs(), m_cps()
        {
            clip(0, width, 0, height);
        }

        void Page::clip(double x0, double x1, double y0, double y1)
        {
            auto cps_count = m_cps.size();
            if (cps_count == 0 || !m_cps.back().equals(x0, x1, y0, y1))
            {
                m_cps.emplace_back(Clip(cps_count, x0, x1, y0, y1));
            }
        }

        void Page::put(std::shared_ptr<DrawCall> dc)
        {
            m_dcs.emplace_back(dc);
            dc->m_clip_id = m_cps.back().id();
        }

        void Page::clear()
        {
            m_dcs.clear();
            m_cps.clear();
            clip(0, width, 0, height);
        }
        void Page::build_svg(std::ostream &os) const
        {
            svg_elem(os, "svg");
            svg_field(os, "xmlns", "http://www.w3.org/2000/svg");
            svg_field(os, "xmlns:xlink", "http://www.w3.org/1999/xlink");
            svg_field(os, "width", width);
            svg_field(os, "height", height);
            os << fmt::format("viewBox=\"0 0 {:.2f} {:.2f}\"", width, height)
               << ">\n<defs>\n"
                  "  <style type='text/css'><![CDATA[\n"
                  "    line, polyline, polygon, path, rect, circle {\n"
                  "      fill: none;\n"
                  "      stroke: #000000;\n"
                  "      stroke-linecap: round;\n"
                  "      stroke-linejoin: round;\n"
                  "      stroke-miterlimit: 10.00;\n"
                  "    }\n"
                  "  ]]></style>\n";

            for (const auto &cp : m_cps)
            {
                cp.build_svg_def(os);
                os << "\n";
            }

            os << "</defs>\n"

               << "<rect width=\"100%\" height=\"100%\" "
                  "style=\"stroke: none; ";
            css_field_color(os, "fill", fill);
            os << "\" />\n";

            for (const auto &dc : m_dcs)
            {
                dc->build_svg(os);
                os << "\n";
            }
            os << "</svg>";
        }

    } // namespace dc
} // namespace httpgd