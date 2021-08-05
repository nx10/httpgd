#include "RendererStrings.h"

namespace httpgd::dc
{

    void RendererStrings::render(const Page &t_page, double t_scale)
    {
        page(t_page);
    }

    std::string RendererStrings::get_string() const
    {
        return fmt::to_string(os);
    }

    void RendererStrings::page(const Page &t_page)
    {
        string_count = 0;
        for (auto it = t_page.dcs.begin(); it != t_page.dcs.end(); ++it)
        {
            (*it)->render(this);
        }
    }

    void RendererStrings::dc(const DrawCall &t_dc)
    {
    }

    void RendererStrings::rect(const Rect &t_rect)
    {
    }

    void RendererStrings::text(const Text &t_text)
    {
        if (string_count++ > 0) {
            fmt::format_to(os, "\n");
        }
        fmt::format_to(os, "{}", t_text.str);
    }

    void RendererStrings::circle(const Circle &t_circle)
    {
    }

    void RendererStrings::line(const Line &t_line)
    {
    }

    void RendererStrings::polyline(const Polyline &t_polyline)
    {
    }

    void RendererStrings::polygon(const Polygon &t_polygon)
    {
    }

    void RendererStrings::path(const Path &t_path)
    {
    }

    void RendererStrings::raster(const Raster &t_raster)
    {
    }

} // namespace httpgd::dc
