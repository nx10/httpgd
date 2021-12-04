#ifndef RENDERER_SVG_H
#define RENDERER_SVG_H

#include "DrawData.h"
#include <fmt/format.h>
#include <boost/optional.hpp>
#include <string>

namespace httpgd::dc
{
    class RendererSVG : public Renderer, public StringRenderingTarget
    {
    public:
        explicit RendererSVG(boost::optional<std::string> t_extra_css);
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
        boost::optional<std::string> m_extra_css;
        double m_scale;
    };

    /**
     * Produces SVG that can directly be embedded in HTML documents 
     * without causing ID conflicts at the expense of larger file size.
     * - Does not use style tags or CDATA embedded CSS.
     * - Appends random UUID to document-wide (clipPath) IDs.
     */
    class RendererSVGPortable : public Renderer, public StringRenderingTarget
    {
    public:
        RendererSVGPortable();
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
        std::string m_unique_id;
    };

    class RendererSVGZ : public RendererSVG, public BinaryRenderingTarget
    {
    public:
        explicit RendererSVGZ(boost::optional<std::string> t_extra_css);
        [[nodiscard]] 
        std::vector<unsigned char> get_binary() const override;
    };
    
    class RendererSVGZPortable : public RendererSVGPortable, public BinaryRenderingTarget
    {
    public:
        RendererSVGZPortable();
        [[nodiscard]] 
        std::vector<unsigned char> get_binary() const override;
    };
    
} // namespace httpgd::dc
#endif // RENDERER_SVG_H