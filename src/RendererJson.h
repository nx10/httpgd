#ifndef RENDERER_JSON_H
#define RENDERER_JSON_H

#include "DrawData.h"
#include <fmt/format.h>

namespace httpgd::dc
{
    class RendererJSON : public Renderer
    {
    public:
        RendererJSON();
        void render(const Page &obj) override;
        std::string to_string();
        void dc(const DrawCall &obj) override;
        void rect(const Rect &obj) override;
        void text(const Text &obj) override;
        void circle(const Circle &obj) override;
        void line(const Line &obj) override;
        void polyline(const Polyline &obj) override;
        void polygon(const Polygon &obj) override;
        void path(const Path &obj) override;
        void raster(const Raster &obj) override;
    
    private:
        fmt::memory_buffer os;
    };
    
} // namespace httpgd::dc
#endif // RENDERER_JSON_H