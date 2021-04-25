#include "RendererJson.h"

namespace httpgd::dc
{
    
    void RendererJSON::render(const Page &t_page) 
    {
        page(t_page);
    }
    
    std::string RendererJSON::get_string() const 
    {
        return fmt::to_string(os);
    }
    
    void RendererJSON::page(const Page &t_page) 
    {
        fmt::format_to(os, "{{\n \"clips\": [\n");
        for (const auto &cp : t_page.cps)
        {
            fmt::format_to(os, R""(  {{ "id": "{}", "x": "{:.2f}", "y": "{:.2f}", "w": "{:.2f}", "h": "{:.2f}" }})"", cp.id, cp.rect.x, cp.rect.y, cp.rect.width, cp.rect.height);
            fmt::format_to(os, ",\n");
        }
        
        fmt::format_to(os, " ],\n \"draw_calls\": [\n");
        for (const auto &dc : t_page.dcs)
        {
            fmt::format_to(os, "  {{ ");
            dc->render(this);
            fmt::format_to(os, " }},\n");
        }
        fmt::format_to(os, " ]\n}}");
    }

    void RendererJSON::dc(const DrawCall &t_dc)
    {
        fmt::format_to(os, "\"type\": \"unknown\"");
    }

    void RendererJSON::rect(const Rect &t_rect)
    {
        fmt::format_to(os, R""("type": "rect", "x": "{:.2f}", "y": "{:.2f}", "w": "{:.2f}", "h": "{:.2f}")"", t_rect.rect.x, t_rect.rect.y, t_rect.rect.width, t_rect.rect.height);
    }

    void RendererJSON::text(const Text &t_text)
    {
        fmt::format_to(os, R""("type": "text", "x": "{:.2f}" "y": "{:.2f}", "str": "{}" )"", t_text.pos.x, t_text.pos.y, t_text.str);
    }

    void RendererJSON::circle(const Circle &t_circle)
    {
        fmt::format_to(os, R""("type": "circle", "x": "{:.2f}" "y": "{:.2f}" "r": "{:.2f}")"", t_circle.pos.x, t_circle.pos.y, t_circle.radius);
    }

    void RendererJSON::line(const Line &t_line)
    {
        fmt::format_to(os, R""("type": "line", "x0": "{:.2f}", "y0": "{:.2f}", "x1": "{:.2f}", "y1": "{:.2f}")"", t_line.orig.x, t_line.orig.y, t_line.dest.x, t_line.dest.y);
    }

    void RendererJSON::polyline(const Polyline &obj)
    {
        fmt::format_to(os, "\"type\": \"polyline\"");
    }

    void RendererJSON::polygon(const Polygon &obj)
    {
        fmt::format_to(os, "\"type\": \"polygon\"");
    }

    void RendererJSON::path(const Path &obj)
    {
        fmt::format_to(os, "\"type\": \"path\"");
    }
    
    void RendererJSON::raster(const Raster &obj)
    {
        fmt::format_to(os, "\"type\": \"raster\"");
    }


} // namespace httpgd::dc
