#include "RendererJson.h"

namespace httpgd::dc
{
    RendererJSON::RendererJSON()
        : os()
    {
    }
    
    void RendererJSON::render(const Page &obj) 
    {
        fmt::format_to(os, "{{\n \"draw_calls\": [\n");
        for (const auto &dc : obj.dcs)
        {
            fmt::format_to(os, "  {{ ");
            dc->render(this);
            fmt::format_to(os, " }},\n");
        }
        fmt::format_to(os, " ]\n}}");
    }
    
    std::string RendererJSON::to_string() 
    {
        return fmt::to_string(os);
    }

    void RendererJSON::dc(const DrawCall &t_dc)
    {
        fmt::format_to(os, "\"type\": \"unknown\"");
    }

    void RendererJSON::rect(const Rect &t_rect)
    {
        fmt::format_to(os, "\"type\": \"rect\"");
    }

    void RendererJSON::text(const Text &obj)
    {
        fmt::format_to(os, "\"type\": \"text\"");
    }

    void RendererJSON::circle(const Circle &t_circle)
    {
        fmt::format_to(os, R""("type": "circle", "x": "{:.2f}" "y": "{:.2f}" "r": "{:.2f}" )"", t_circle.pos.x, t_circle.pos.y, t_circle.radius);
    }

    void RendererJSON::line(const Line &obj)
    {
        fmt::format_to(os, "\"type\": \"line\"");
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
