#ifndef RENDERER_JSON_H
#define RENDERER_JSON_H

#include "DrawData.h"
#include <fmt/format.h>

namespace httpgd::dc
{
    class RendererJSON : public StringRenderingTarget, public Renderer
    {
    public:
        void render(const Page &t_page, double t_scale) override;
        [[nodiscard]]
        std::string get_string() const override;

        // Renderer
        void page(const Page &t_page) override;
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
        fmt::memory_buffer os;
        double m_scale;
    };
    
} // namespace httpgd::dc
#endif // RENDERER_JSON_H