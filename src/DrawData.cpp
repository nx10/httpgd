
#include "DrawData.h"

#include <cmath>
#include <string>
#include <vector>

namespace httpgd::dc
{
    void DrawCall::render(Renderer *t_renderer) const
    {
        t_renderer->dc(*this);
    }
    void Text::render(Renderer *t_renderer) const
    {
        t_renderer->text(*this);
    }
    void Line::render(Renderer *t_renderer) const
    {
        t_renderer->line(*this);
    }
    void Polyline::render(Renderer *t_renderer) const
    {
        t_renderer->polyline(*this);
    }
    void Polygon::render(Renderer *t_renderer) const
    {
        t_renderer->polygon(*this);
    }
    void Raster::render(Renderer *t_renderer) const
    {
        t_renderer->raster(*this);
    }
    void Path::render(Renderer *t_renderer) const
    {
        t_renderer->path(*this);
    }
    void Circle::render(Renderer *t_renderer) const
    {
        t_renderer->circle(*this);
    }
    void Rect::render(Renderer *t_renderer) const
    {
        t_renderer->rect(*this);
    }

    Text::Text(color_t t_col, gvertex<double> t_pos, std::string &&t_str, double t_rot, double t_hadj, TextInfo &&t_text)
        : col(t_col), pos(t_pos), rot(t_rot), hadj(t_hadj), str(t_str), text(t_text)
    {
    }
    Circle::Circle(LineInfo &&t_line, color_t t_fill, gvertex<double> t_pos, double t_radius)
        : line(t_line), fill(t_fill), pos(t_pos), radius(t_radius)
    {
    }
    Line::Line(LineInfo &&t_line, gvertex<double> t_orig, gvertex<double> t_dest)
        : line(t_line), orig(t_orig), dest(t_dest)
    {
    }
    Rect::Rect(LineInfo &&t_line, color_t t_fill, grect<double> t_rect)
        : line(t_line), fill(t_fill), rect(t_rect)
    {
    }
    Polyline::Polyline(LineInfo &&t_line, std::vector<gvertex<double>> &&t_points)
        : line(t_line), points(t_points)
    {
    }
    Polygon::Polygon(LineInfo &&t_line, color_t t_fill, std::vector<gvertex<double>> &&t_points)
        : line(t_line), fill(t_fill), points(t_points)
    {
    }
    Path::Path(LineInfo &&t_line, color_t t_fill, std::vector<gvertex<double>> &&t_points, std::vector<int> &&t_nper, bool t_winding)
        : line(t_line), fill(t_fill), points(t_points), nper(t_nper), winding(t_winding)
    {
    }
    Raster::Raster(std::vector<unsigned int> &&t_raster, gvertex<int> t_wh,
               grect<double> t_rect,
               double t_rot,
               bool t_interpolate)
        : raster(t_raster), wh(t_wh), rect(t_rect), rot(t_rot), interpolate(t_interpolate)
    {
    }

    Clip::Clip(clip_id_t t_id, grect<double> t_rect)
        : id(t_id), rect(t_rect)
    {
    }
    
    bool Clip::equals(grect<double> t_rect) const
    {
        return rect_equals(t_rect, rect, 0.01);
    }

    Page::Page(page_id_t t_id, gvertex<double> t_size)
        : id(t_id), size(t_size)
    {
        clip({0, 0, size.x, size.y});
    }

    void Page::clip(grect<double> t_rect)
    {
        const auto cps_count = cps.size();
        if (cps_count == 0 || !cps.back().equals(t_rect))
        {
            cps.emplace_back(Clip(cps_count, t_rect));
        }
    }

    void Page::put(std::shared_ptr<DrawCall> t_dc)
    {
        dcs.emplace_back(t_dc);
        t_dc->clip_id = cps.back().id;
    }

    void Page::clear()
    {
        dcs.clear();
        cps.clear();
        clip({0, 0, size.x, size.y});
    }

} // namespace httpgd::dc