#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <gdtools.h>

#include "drawdata.h"

#define R_RGB(r, g, b) ((r) | ((g) << 8) | ((b) << 16) | 0xFF000000)
#define R_RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define R_RED(col) (((col)) & 255)
#define R_GREEN(col) (((col) >> 8) & 255)
#define R_BLUE(col) (((col) >> 16) & 255)
#define R_ALPHA(col) (((col) >> 24) & 255)
#define R_OPAQUE(col) (R_ALPHA(col) == 255)
#define R_TRANSPARENT(col) (R_ALPHA(col) == 0)
#define R_TRANWHITE (R_RGBA(255, 255, 255, 0))
#define R_BLACK (R_RGBA(0, 0, 0, 255))

#define LTY_BLANK -1
#define LTY_SOLID 0
#define LTY_DASHED 4 + (4 << 4)
#define LTY_DOTTED 1 + (3 << 4)
#define LTY_DOTDASH 1 + (3 << 4) + (4 << 8) + (3 << 12)
#define LTY_LONGDASH 7 + (3 << 4)
#define LTY_TWODASH 2 + (2 << 4) + (6 << 8) + (2 << 12)

namespace httpgd
{
  namespace dc
  {

    template <typename T>
    inline void svg_field(std::string &buf, const std::string &name, T val)
    {
      buf.append(name).append("=\"").append(std::to_string(val)).append("\" ");
    }
    template <>
    inline void svg_field<const char *>(std::string &buf, const std::string &name, const char *val)
    {
      buf.append(name).append("=\"").append(val).append("\" ");
    }
    template <>
    inline void svg_field<std::string>(std::string &buf, const std::string &name, std::string val)
    {
      buf.append(name).append("=\"").append(val).append("\" ");
    }

    inline void svg_elem(std::string &buf, const std::string &name)
    {
      buf.append("<").append(name).append(" ");
    }

    template <typename T>
    inline void css_field(std::string &buf, const std::string &name, T val)
    {
      buf.append(name).append(": ").append(std::to_string(val)).append(";");
    }
    template <>
    inline void css_field<const char *>(std::string &buf, const std::string &name, const char *val)
    {
      buf.append(name).append(": ").append(val).append(";");
    }

    inline void css_color(std::string &buf, int c)
    {
      char hexcol[17];
      snprintf(hexcol, sizeof hexcol, "#%02X%02X%02X", R_RED(c), R_GREEN(c), R_BLUE(c));
      buf.append(hexcol);
    }
    inline void css_field_color(std::string &buf, const std::string &name, int c)
    {
      buf.append(name).append(": ");
      css_color(buf, c);
      buf.append(";");
    }

    inline void write_style_col(std::string &buf, const DrawCall *dc)
    {
      buf.append("fill: ");
      int col = dc->m_fill;
      int alpha = R_ALPHA(col);
      if (alpha == 0)
      {
        buf += ": none;";
      }
      else
      {
        css_color(buf, col);
        buf += ";";
        if (alpha != 255)
          buf.append(" fill-opacity: ").append(std::to_string(alpha / 255.0)).append(";");
      }
    }

    inline double scale_lty(int lty, double lwd)
    {
      // Don't rescale if lwd < 1
      // https://github.com/wch/r-source/blob/master/src/library/grDevices/src/cairo/cairoFns.c#L134
      return ((lwd > 1) ? lwd : 1) * (lty & 15);
    }
    inline void write_style_linetype(std::string &buf, const DrawCall *dc)
    {
      int lty = dc->m_lty;

      // 1 lwd = 1/96", but units in rest of document are 1/72"
      css_field(buf, "stroke-width", dc->m_lwd / 96.0 * 72);

      // Default is "stroke: #000000;" as declared in <style>
      if (dc->m_col != R_BLACK)
        css_field_color(buf, "stroke", dc->m_col);

      // Set line pattern type
      switch (lty)
      {
      case LTY_BLANK: // never called: blank lines never get to this point
      case LTY_SOLID: // default svg setting, so don't need to write out
        break;
      default:
        // For details
        // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
        buf += " stroke-dasharray: ";
        // First number
        buf += std::to_string(scale_lty(lty, dc->m_lwd));
        lty = lty >> 4;
        // Remaining numbers
        for (int i = 1; i < 8 && lty & 15; i++)
        {
          buf += "," + std::to_string(scale_lty(lty, dc->m_lwd));
          lty = lty >> 4;
        }
        buf += ";";
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
        if (std::abs(dc->m_lmitre - 10.0) > 1e-3) // 10 is declared to be the default in <style>
          css_field(buf, "stroke-miterlimit", dc->m_lmitre);
        break;
      default:
        break;
      }
    }

    // DRAW CALL OBJECTS

    DrawCall::DrawCall()
    {
    }
    DrawCall::~DrawCall() {}
    void DrawCall::to_svg(std::string &buf)
    {
      buf.append("<!-- unknown draw call -->");
    }

    Text::Text(double x, double y,
               const std::string &str,
               double rot, double hadj)
        : DrawCall(),
          m_x(x), m_y(y), m_rot(rot), m_hadj(hadj), m_str(str)
    {
    }
    void Text::to_svg(std::string &buf)
    {
      svg_elem(buf, "text");
      if (m_rot == 0.0)
      {
        svg_field(buf, "x", m_x);
        svg_field(buf, "y", m_y);
      }
      else
      {
        buf.append("transform=\"translate(");
        buf.append(std::to_string(m_x));
        buf.append(",");
        buf.append(std::to_string(m_y));
        buf.append(") rotate(");
        buf.append(std::to_string(m_rot * -1.0));
        buf.append(")\" ");
      }
      svg_field(buf, "font-family", m_font_family);
      svg_field(buf, "font-size", m_fontsize);
      if (m_bold)
      {
        svg_field(buf, "font-weight", "bold");
      }
      if (m_italic)
      {
        svg_field(buf, "font-style", "italic");
      }
      buf.append(">").append(m_str).append("</text>");
    }

    Circle::Circle(double x, double y, double r)
        : DrawCall(),
          m_x(x), m_y(y), m_r(r)
    {
    }
    void Circle::to_svg(std::string &buf)
    {
      svg_elem(buf, "circle");
      svg_field(buf, "cx", m_x);
      svg_field(buf, "cy", m_y);
      svg_field(buf, "r", m_r);

      std::string style;
      write_style_linetype(style, this);
      if (R_ALPHA(m_fill) != 0)
        write_style_col(style, this);
      svg_field(buf, "style", style);

      buf.append("/>");
    }

    Line::Line(double x1, double y1, double x2, double y2)
        : DrawCall(),
          m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
    {
    }
    void Line::to_svg(std::string &buf)
    {
      svg_elem(buf, "line");
      svg_field(buf, "x1", m_x1);
      svg_field(buf, "y1", m_y1);
      svg_field(buf, "x2", m_x2);
      svg_field(buf, "y2", m_y2);

      std::string style;
      write_style_linetype(style, this);
      svg_field(buf, "style", style);

      buf.append("/>");
    }

    Rect::Rect(double x0, double y0, double x1, double y1)
        : DrawCall(),
          m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1)
    {
    }
    void Rect::to_svg(std::string &buf)
    {
      svg_elem(buf, "rect");
      svg_field(buf, "x", std::min(m_x0, m_x1));
      svg_field(buf, "y", std::min(m_y0, m_y1));
      svg_field(buf, "width", std::abs(m_x0 - m_x1));
      svg_field(buf, "height", std::abs(m_y0 - m_y1));

      std::string style;
      write_style_linetype(style, this);
      if (R_ALPHA(m_fill) != 0)
        write_style_col(style, this);
      svg_field(buf, "style", style);

      buf.append("/>");
    }

    Polyline::Polyline(int n, std::vector<double> &x, std::vector<double> &y)
        : DrawCall(),
          m_n(n), m_x(x), m_y(y)
    {
    }
    void Polyline::to_svg(std::string &buf)
    {
      svg_elem(buf, "polyline");
      std::string pts = "";
      for (int i = 0; i < m_n; i++)
      {
        pts += std::to_string(m_x[i]).append(",");
        pts += std::to_string(m_y[i]).append(" ");
      }
      svg_field(buf, "points", pts);

      std::string style;
      write_style_linetype(style, this);
      svg_field(buf, "style", style);

      buf.append("/>");
    }
    Polygon::Polygon(int n, std::vector<double> &x, std::vector<double> &y)
        : DrawCall(),
          m_n(n), m_x(x), m_y(y)
    {
    }
    void Polygon::to_svg(std::string &buf)
    {
      svg_elem(buf, "polygon");
      std::string pts = "";
      for (int i = 0; i < m_n; i++)
      {
        pts += std::to_string(m_x[i]).append(",");
        pts += std::to_string(m_y[i]).append(" ");
      }
      svg_field(buf, "points", pts);

      std::string style;
      write_style_linetype(style, this);
      if (R_ALPHA(m_fill) != 0)
        write_style_col(style, this);
      svg_field(buf, "style", style);

      buf.append("/>");
    }
    Path::Path(std::vector<double> &x, std::vector<double> &y, int npoly, std::vector<int> &nper, bool winding)
        : DrawCall(),
          m_x(x), m_y(y), m_npoly(npoly), m_nper(nper), m_winding(winding)
    {
    }
    void Path::to_svg(std::string &buf)
    {
    }

    Raster::Raster(std::vector<unsigned int> &raster, int w, int h,
                   double x, double y,
                   double width, double height,
                   double rot,
                   bool interpolate)
        : DrawCall(),
          m_raster(raster), m_w(w), m_h(h), m_x(x), m_y(y), m_width(width), m_height(height), m_rot(rot), m_interpolate(interpolate)
    {
    }
    void Raster::to_svg(std::string &buf)
    {
      double imageHeight = m_height;
      double imageWidth = m_width;

      if (m_height < 0)
        imageHeight = -m_height;
      if (m_width < 0)
        imageWidth = -m_width;

      svg_elem(buf, "image");
      svg_field(buf, "width", imageWidth);
      svg_field(buf, "height", imageHeight);
      svg_field(buf, "x", m_x);
      svg_field(buf, "y", m_y - imageHeight);
      if (m_rot != 0)
      {
        buf.append("transform=\"rotate(");
        buf.append(std::to_string(-1.0 * m_rot));
        buf.append(",");
        buf.append(std::to_string(m_x));
        buf.append(",");
        buf.append(std::to_string(m_y));
        buf.append(")\" ");
      }
      buf.append(" xlink:href=\"data:image/png;base64,");
      buf.append(gdtools::raster_to_str(m_raster, m_w, m_h, imageWidth, imageHeight, m_interpolate));
      buf.append("\"/>");
    }

    Page::Page(double width, double height)
        : m_width(width), m_height(height), m_dcs(), m_upid(0)
    {
    }
    Page::~Page()
    {
      for (DrawCall *p : m_dcs)
      {
        delete p;
      }
    }

    void Page::put(DrawCall *dc)
    {
      m_dcs.push_back(dc);
      m_upid++;
    }

    void Page::clear()
    {
      for (DrawCall *p : m_dcs)
      {
        delete p;
      }
      m_dcs.clear();
      m_upid++;
    }
    int Page::get_upid()
    {
      return m_upid;
    }
    void Page::to_svg(std::string &buf)
    {
      svg_elem(buf, "svg");
      svg_field(buf, "xmlns", "http://www.w3.org/2000/svg");
      svg_field(buf, "xmlns:xlink", "http://www.w3.org/1999/xlink");
      svg_field(buf, "width", std::to_string(m_width));
      svg_field(buf, "height", std::to_string(m_height));
      buf.append("viewBox=\"0 0 ");
      buf.append(std::to_string(m_width)).append(" ").append(std::to_string(m_height));
      buf.append("\"");

      buf += "><defs>"
             "  <style type='text/css'><![CDATA["
             "    line, polyline, polygon, path, rect, circle {"
             "      fill: none;"
             "      stroke: #000000;"
             "      stroke-linecap: round;"
             "      stroke-linejoin: round;"
             "      stroke-miterlimit: 10.00;"
             "    }"
             "  ]]></style>"
             "</defs>\n";

      buf += "<rect width='100%' height='100%' "
             "style=\"stroke: none; ";
      css_field_color(buf, "fill", m_fill);
      buf += "\"/>\n";

      std::for_each(m_dcs.begin(), m_dcs.end(), [&](DrawCall *piece) { buf.append("  "); piece->to_svg(buf); buf.append("\n"); });
      buf += "\n</svg>";
    }

  } // namespace dc
} // namespace httpgd