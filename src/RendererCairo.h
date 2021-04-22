#ifndef RENDERER_CAIRO_H
#define RENDERER_CAIRO_H

#include "DrawData.h"

#include <cairo.h>
#include <fmt/format.h>
#include <vector>

namespace httpgd::dc
{
    class RendererCairo : public TargetRenderer<std::vector<unsigned char>>
    {
    public:
        void render(const Page &t_page) override;
        [[nodiscard]] std::vector<unsigned char> get() const override;
        void dc(const DrawCall &t_dc) override;
        void rect(const Rect &t_rect) override;
        void text(const Text &t_text) override;
        void circle(const Circle &t_circle) override;
        void line(const Line &t_line) override;
        void polyline(const Polyline &t_polyline) override;
        void polygon(const Polygon &t_polygon) override;
        void path(const Path &t_path) override;
        void raster(const Raster &t_raster) override;

    private:
        std::vector<unsigned char> m_render_data{};
        cairo_surface_t *surface = nullptr;
        cairo_t *cr = nullptr;
    };

} // namespace httpgd::dc
#endif // RENDERER_CAIRO_H