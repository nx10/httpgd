#ifndef RENDERER_SVG_H
#define RENDERER_SVG_H

#include "DrawData.h"
#include <fmt/format.h>
#include <boost/optional.hpp>

namespace httpgd::dc
{
    class RendererSVG : public Renderer
    {
    public:
        explicit RendererSVG(boost::optional<std::string> t_extra_css);
        void render(const Page &t_page) override;
        std::string to_string();
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
        boost::optional<std::string> m_extra_css;
    };
    
} // namespace httpgd::dc
#endif // RENDERER_SVG_H