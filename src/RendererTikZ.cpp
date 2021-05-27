#include "RendererTikZ.h"

namespace httpgd::dc
{
    static inline void write_tex_escaped(fmt::memory_buffer &os, const std::string &text)
    {
        for (const char &c : text)
        {
            switch (c)
            {
            case '&':
                fmt::format_to(os, "\\&");
                break;
            case '%':
                fmt::format_to(os, "\\%");
                break;
            case '$':
                fmt::format_to(os, "\\$");
                break;
            case '#':
                fmt::format_to(os, "\\#");
                break;
            case '_':
                fmt::format_to(os, "\\_");
                break;
            case '{':
                fmt::format_to(os, "\\{");
                break;
            case '}':
                fmt::format_to(os, "\\}");
                break;
            case '~':
                fmt::format_to(os, "\\textasciitilde");
                break;
            case '^':
                fmt::format_to(os, "\\textasciicircum");
                break;
            case '\\':
                fmt::format_to(os, "\\textbackslash");
                break;
            default:
                fmt::format_to(os, "{}", c);
            }
        }
    }

    static inline void tex_xcolor_rgb(fmt::memory_buffer &os, color_t col)
    {
        fmt::format_to(os, "{{rgb,255:red,{}; green,{}; blue,{}}}",
                       color::red(col), color::green(col), color::blue(col));
    }
    
    static inline void tex_fill_or_omit(fmt::memory_buffer &os, color_t col)
    {
        auto alpha = color::alpha(col);
        if (alpha != 0)
        {
            fmt::format_to(os, "fill=");
            tex_xcolor_rgb(os, col);
            fmt::format_to(os, ",");
            if (alpha != color::byte_mask)
            {
                fmt::format_to(os, "fill opacity={:.2f},", alpha / (double) color::byte_mask);
            }
        }
    }

    static inline void tex_lineinfo(fmt::memory_buffer &os, const LineInfo &line)
    {
        // 1 lwd = 1/96", but units in rest of document are 1/72"
        fmt::format_to(os, "line width={:.2f}pt", line.lwd / 96.0 * 72);

        if (line.col != color::rgba(0, 0, 0, 255))
        {
            int alpha = color::alpha(line.col);
            if (alpha == 0)
            {
                fmt::format_to(os, ",draw=none");
            }
            else
            {
                fmt::format_to(os, ",draw=");
                tex_xcolor_rgb(os, line.col);
                if (alpha != 255)
                {
                    fmt::format_to(os, ",fill opacity={:.2f}", alpha / 255.0);
                }
            }
        }

        // Set line pattern type
        int lty = line.lty;
        switch (lty)
        {
        case LineInfo::LTY::BLANK: // never called: blank lines never get to this point
        case LineInfo::LTY::SOLID: // default svg setting, so don't need to write out
            break;
        default:
            // For details
            // https://github.com/wch/r-source/blob/trunk/src/include/R_ext/GraphicsEngine.h#L337
            fmt::format_to(os, ",dash pattern=on {}", lty & 15);
            lty = lty >> 4;
            // Remaining numbers
            for (int i = 1; i < 8 && lty & 15; i++)
            {
                fmt::format_to(os, " {} {}", (i % 2 == 0) ? "on" : "off", lty & 15);
                lty = lty >> 4;
            }
            break;
        }

        // Set line end shape
        switch (line.lend)
        {
        case LineInfo::GC_ROUND_CAP:
            fmt::format_to(os, ",line cap=round");
            break;
        case LineInfo::GC_BUTT_CAP:
            break;
        case LineInfo::GC_SQUARE_CAP:
            fmt::format_to(os, ",line cap=rect");
            break;
        default:
            break;
        }

        // Set line join shape
        switch (line.ljoin)
        {
        case LineInfo::GC_ROUND_JOIN:
            fmt::format_to(os, ",line join=round");
            break;
        case LineInfo::GC_BEVEL_JOIN:
            fmt::format_to(os, ",line join=bevel");
            break;
        case LineInfo::GC_MITRE_JOIN:
            if (std::abs(line.lmitre - 10.0) > 1e-3)
            {
                fmt::format_to(os, ",miter limit={:.2f}", line.lmitre);
            }
            break;
        default:
            break;
        }
    }

    void RendererTikZ::render(const Page &t_page, double t_scale)
    {
        m_scale = t_scale;
        page(t_page);
    }

    std::string RendererTikZ::get_string() const
    {
        return fmt::to_string(os);
    }

    void RendererTikZ::page(const Page &t_page)
    {
        fmt::format_to(os, R""(\begin{{tikzpicture}}[x=1pt,y=-1pt,scale={:.2f}])""
                           "\n",
                       m_scale);

        auto bg_alpha = color::alpha(t_page.fill);
        if (bg_alpha != 0)
        {
            fmt::format_to(os, R""(\fill[fill=)"");
            tex_xcolor_rgb(os, t_page.fill);
            if (bg_alpha != color::byte_mask)
            {
                fmt::format_to(os, ",fill opacity={:.2f}", color::byte_frac(bg_alpha));
            }
            fmt::format_to(os, R""(] (0,0) rectangle ({:.2f},{:.2f});)"" "\n", t_page.size.x, t_page.size.y);
        }

        const auto &first_clip = t_page.cps.front();
        fmt::format_to(os, R""(\begin{{scope}}\clip ({:.2f},{:.2f}) rectangle ({:.2f},{:.2f});)""
                           "\n",
                       first_clip.rect.x, first_clip.rect.y, first_clip.rect.x + first_clip.rect.width, first_clip.rect.y + first_clip.rect.height);
        auto last_clip_id = first_clip.id;
        for (auto it = t_page.dcs.begin(); it != t_page.dcs.end(); ++it)
        {
            if (it != t_page.dcs.begin())
            {
                fmt::format_to(os, "\n");
            }
            if ((*it)->clip_id != last_clip_id)
            {
                const auto &next_clip = *std::find_if(t_page.cps.begin(), t_page.cps.end(), [&](const Clip &clip) {
                    return clip.id == (*it)->clip_id;
                });
                fmt::format_to(os, R""(\end{{scope}}\begin{{scope}}\clip ({:.2f},{:.2f}) rectangle ({:.2f},{:.2f});)""
                                   "\n",
                               next_clip.rect.x, next_clip.rect.y, next_clip.rect.x + next_clip.rect.width, next_clip.rect.y + next_clip.rect.height);
                last_clip_id = next_clip.id;
            }
            (*it)->render(this);
        }
        fmt::format_to(os, "\n"
                           R""(\end{{scope}})""
                           "\n"
                           R""(\end{{tikzpicture}})"");
    }

    void RendererTikZ::dc(const DrawCall &t_dc)
    {
    }

    void RendererTikZ::rect(const Rect &t_rect)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_fill_or_omit(os, t_rect.fill);
        tex_lineinfo(os, t_rect.line);
        fmt::format_to(os, R""(] ({:.2f},{:.2f}) rectangle ({:.2f},{:.2f});)"", t_rect.rect.x, t_rect.rect.y, t_rect.rect.x + t_rect.rect.width, t_rect.rect.y + t_rect.rect.height);
    }

    void RendererTikZ::text(const Text &t_text)
    {
        fmt::format_to(os, R""(\node[text=)"");
        tex_xcolor_rgb(os, t_text.col);
        if (!color::opaque(t_text.col))
        {
            fmt::format_to(os, ",text opacity={:.2f}", color::alpha(t_text.col) / 255.0);
        }

        if (t_text.rot > 0)
        {
            fmt::format_to(os, R""(,rotate={:.2f})"", t_text.rot);
        }

        fmt::format_to(os, ",anchor=");

        if (std::fabs(t_text.hadj - 0.5) < 0.1)
        {
            fmt::format_to(os, "base");
        }
        else if (std::fabs(t_text.hadj - 1) < 0.1)
        {
            fmt::format_to(os, "base east");
        }
        else
        {
            fmt::format_to(os, "base west");
        }

        fmt::format_to(os, R""(,inner sep=0pt, outer sep=0pt, scale={:.2f}] at ({:.2f},{:.2f}) {{\fontsize{{{:.2f}}}{{\baselineskip}}\selectfont )"",
                       m_scale, t_text.pos.x, t_text.pos.y, t_text.text.fontsize);
        write_tex_escaped(os, t_text.str);
        fmt::format_to(os, R""(}};)"");
    }

    void RendererTikZ::circle(const Circle &t_circle)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_fill_or_omit(os, t_circle.fill);
        tex_lineinfo(os, t_circle.line);
        fmt::format_to(os, R""(] ({:.2f},{:.2f}) circle ({:.2f});)"", t_circle.pos.x, t_circle.pos.y, t_circle.radius);
    }

    void RendererTikZ::line(const Line &t_line)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_lineinfo(os, t_line.line);
        fmt::format_to(os, R""(] ({:.2f},{:.2f}) -- ({:.2f},{:.2f});)"", t_line.orig.x, t_line.orig.y, t_line.dest.x, t_line.dest.y);
    }

    void RendererTikZ::polyline(const Polyline &t_polyline)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_lineinfo(os, t_polyline.line);
        fmt::format_to(os, R""(] )"");
        for (auto it = t_polyline.points.begin(); it != t_polyline.points.end(); ++it)
        {
            if (it != t_polyline.points.begin())
            {
                fmt::format_to(os, " -- ");
            }
            fmt::format_to(os, "({:.2f},{:.2f})", it->x, it->y);
        }
        fmt::format_to(os, ";");
    }

    void RendererTikZ::polygon(const Polygon &t_polygon)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_fill_or_omit(os, t_polygon.fill);
        tex_lineinfo(os, t_polygon.line);
        fmt::format_to(os, R""(] )"");
        for (auto it = t_polygon.points.begin(); it != t_polygon.points.end(); ++it)
        {
            fmt::format_to(os, "({:.2f},{:.2f}) -- ", it->x, it->y);
        }
        fmt::format_to(os, "cycle;");
    }

    void RendererTikZ::path(const Path &t_path)
    {
        fmt::format_to(os, R""(\draw[)"");
        tex_fill_or_omit(os, t_path.fill);
        tex_lineinfo(os, t_path.line);
        fmt::format_to(os, R""(] )"");
        auto it_poly = t_path.nper.begin();
        std::size_t left = 0;
        for (auto it = t_path.points.begin(); it != t_path.points.end(); ++it)
        {
            if (left == 0)
            {
                left = (*it_poly) - 1;
                ++it_poly;
                fmt::format_to(os, "({:.2f},{:.2f})", it->x, it->y);
            }
            else
            {
                --left;
                fmt::format_to(os, " -- ({:.2f},{:.2f})", it->x, it->y);

                if (left == 0)
                {
                    fmt::format_to(os, " -- cycle ");
                }
            }
        }
        fmt::format_to(os, ";");
    }

    void RendererTikZ::raster(const Raster &t_raster)
    {
        fmt::format_to(os, R""(% WARNING: TikZ raster image drawing not yet supported.)"");
    }
}