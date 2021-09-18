#include "RendererJson.h"

#include "Base64.h"

namespace httpgd::dc
{

    static inline std::string hexcol(color_t t_color)
    {
        return fmt::format("#{:02X}{:02X}{:02X}", color::red(t_color), color::green(t_color), color::blue(t_color));
    }

    static inline std::string json_lineinfo(const LineInfo &t_line)
    {
        return fmt::format(R""({{ "col": "{}", "lwd": {:.2f}, "lty": {}, "lend": {}, "ljoin": {}, "lmitre": {} }})"",
                           hexcol(t_line.col), t_line.lwd, t_line.lty, t_line.lend, t_line.ljoin, t_line.lmitre);
    }

    static inline void json_verts(fmt::memory_buffer &os, const std::vector<httpgd::gvertex<double>> &t_verts)
    {
        fmt::format_to(os, "[");
        for (auto it = t_verts.begin(); it != t_verts.end(); ++it)
        {
            if (it != t_verts.begin())
            {
                fmt::format_to(os, ", ");
            }
            fmt::format_to(os, "[ {:.2f}, {:.2f} ]", it->x, it->y);
        }
        fmt::format_to(os, "]");
    }

    void RendererJSON::render(const Page &t_page, double t_scale)
    {
        m_scale = t_scale;
        page(t_page);
    }

    std::string RendererJSON::get_string() const
    {
        return fmt::to_string(os);
    }

    void RendererJSON::page(const Page &t_page)
    {
        fmt::format_to(os, "{{\n " R""("id": "{}", "w": {:.2f}, "h": {:.2f}, "scale": {:.2f}, "fill": "{}",)"" "\n",
        t_page.id, t_page.size.x, t_page.size.y, m_scale, hexcol(t_page.fill));
        fmt::format_to(os, " \"clips\": [\n  ");
        for (auto it = t_page.cps.begin(); it != t_page.cps.end(); ++it)
        {
            if (it != t_page.cps.begin())
            {
                fmt::format_to(os, ",\n  ");
            }
            fmt::format_to(os, R""({{ "id": {}, "x": {:.2f}, "y": {:.2f}, "w": {:.2f}, "h": {:.2f} }})"",
                           it->id, it->rect.x, it->rect.y, it->rect.width, it->rect.height);
        }

        fmt::format_to(os, "\n ],\n \"draw_calls\": [\n  ");
        for (auto it = t_page.dcs.begin(); it != t_page.dcs.end(); ++it)
        {
            if (it != t_page.dcs.begin())
            {
                fmt::format_to(os, ",\n  ");
            }
            fmt::format_to(os, "{{ ");
            (*it)->render(this);
            fmt::format_to(os, " }}");
        }
        fmt::format_to(os, "\n ]\n}}");
    }

    void RendererJSON::dc(const DrawCall &t_dc)
    {
        fmt::format_to(os, "\"type\": \"unknown\"");
    }

    void RendererJSON::rect(const Rect &t_rect)
    {
        fmt::format_to(os, R""("type": "rect", "clip_id": {}, "x": {:.2f}, "y": {:.2f}, "w": {:.2f}, "h": {:.2f}, "line": {})"",
                       t_rect.clip_id, t_rect.rect.x, t_rect.rect.y, t_rect.rect.width, t_rect.rect.height, json_lineinfo(t_rect.line));
    }

    void RendererJSON::text(const Text &t_text)
    {
        fmt::format_to(os, R""("type": "text", "clip_id": {}, "x": {:.2f}, "y": {:.2f}, "rot": {:.2f}, "hadj": {:.2f}, "col": "{}", "str": "{}", )""
                           R""("weight": {}, "features": "{}", "font_family": "{}", "fontsize": {:.2f}, "italic": {}, "txtwidth_px": {:.2f})"",
                       t_text.clip_id, t_text.pos.x, t_text.pos.y, t_text.rot, t_text.hadj, hexcol(t_text.col), t_text.str,
                       t_text.text.weight, t_text.text.features, t_text.text.font_family, t_text.text.fontsize, t_text.text.italic, t_text.text.txtwidth_px);
    }

    void RendererJSON::circle(const Circle &t_circle)
    {
        fmt::format_to(os, R""("type": "circle", "clip_id": {}, "x": {:.2f}, "y": {:.2f}, "r": {:.2f}, "fill": "{}", "line": {})"",
                       t_circle.clip_id, t_circle.pos.x, t_circle.pos.y, t_circle.radius, hexcol(t_circle.fill), json_lineinfo(t_circle.line));
    }

    void RendererJSON::line(const Line &t_line)
    {
        fmt::format_to(os, R""("type": "line", "clip_id": {}, "x0": {:.2f}, "y0": {:.2f}, "x1": {:.2f}, "y1": {:.2f}, "line": {})"",
                       t_line.clip_id, t_line.orig.x, t_line.orig.y, t_line.dest.x, t_line.dest.y, json_lineinfo(t_line.line));
    }

    void RendererJSON::polyline(const Polyline &t_polyline)
    {
        fmt::format_to(os, R""("type": "polyline", "clip_id": {}, "line": {}, "points": )"",
                       t_polyline.clip_id, json_lineinfo(t_polyline.line));
        json_verts(os, t_polyline.points);
    }

    void RendererJSON::polygon(const Polygon &t_polygon)
    {
        fmt::format_to(os, R""("type": "polygon", "clip_id": {}, "fill": "{}", "line": {}, "points": )"",
                       t_polygon.clip_id, hexcol(t_polygon.fill), json_lineinfo(t_polygon.line));
        json_verts(os, t_polygon.points);
    }

    void RendererJSON::path(const Path &t_path)
    {
        fmt::format_to(os, R""("type": "path", "clip_id": {}, "fill": "{}", "line": {}, "nper": )"",
                       t_path.clip_id, hexcol(t_path.fill), json_lineinfo(t_path.line));

        fmt::format_to(os, "[");
        for (auto it = t_path.nper.begin(); it != t_path.nper.end(); ++it)
        {
            if (it != t_path.nper.begin())
            {
                fmt::format_to(os, ", ");
            }
            fmt::format_to(os, "{}", *it);
        }
        fmt::format_to(os, R""(], "points": )"");
        json_verts(os, t_path.points);
    }

    void RendererJSON::raster(const Raster &t_raster)
    {
        fmt::format_to(os, R""("type": "raster", "clip_id": {}, "x": {:.2f}, "y": {:.2f}, "w": {:.2f}, "h": {:.2f}, "rot": {:.2f}, "raster": {{ "w": {}, "h": {}, "data": "{}" }})"",
                       t_raster.clip_id, t_raster.rect.x, t_raster.rect.y, t_raster.rect.width, t_raster.rect.height, t_raster.rot, t_raster.wh.x, t_raster.wh.y, raster_base64(t_raster));
    }

} // namespace httpgd::dc
