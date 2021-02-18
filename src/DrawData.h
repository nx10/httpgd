#ifndef HTTPGD_DRAWDATA_H
#define HTTPGD_DRAWDATA_H

#include "HttpgdGeom.h"

#include <algorithm>
#include <boost/optional.hpp>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

// Do not include any R headers here !

namespace httpgd::dc
{
    using clip_id_t = int;
    using page_id_t = int32_t;

    // Data

    struct LineInfo
    {
        enum GC_lineend
        {
            GC_ROUND_CAP = 1,
            GC_BUTT_CAP = 2,
            GC_SQUARE_CAP = 3
        };

        enum GC_linejoin
        {
            GC_ROUND_JOIN = 1,
            GC_MITRE_JOIN = 2,
            GC_BEVEL_JOIN = 3
        };

        color_t col;
        double lwd;
        int lty;
        GC_lineend lend;
        GC_linejoin ljoin;
        double lmitre;
    };

    struct TextInfo
    {
        int weight;
        std::string features;
        std::string font_family;
        double fontsize;
        bool italic;
        double txtwidth_px;
    };

    // Draw calls

    class Clip;

    class DrawCall
    {
    public:
        virtual void svg(std::ostream &os) const;
        [[nodiscard]] clip_id_t clip_id() const;
        void clip_id(clip_id_t t_clip_id);

    private:
        clip_id_t m_clip_id = 0;
    };

    class Text : public DrawCall
    {
    public:
        Text(color_t t_col, vertex<double> t_pos, std::string &&t_str, double t_rot, double t_hadj, TextInfo &&t_text);
        void svg(std::ostream &os) const override;

    private:
        color_t m_col;
        vertex<double> m_pos;
        double m_rot, m_hadj;
        std::string m_str;
        TextInfo m_text;
    };

    class Circle : public DrawCall
    {
    public:
        Circle(LineInfo &&t_line, color_t t_fill, vertex<double> t_pos, double t_radius);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        color_t m_fill;
        vertex<double> m_pos;
        double m_radius;
    };

    class Line : public DrawCall
    {
    public:
        Line(LineInfo &&t_line, vertex<double> t_orig, vertex<double> t_dest);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        vertex<double> m_orig, m_dest;
    };

    class Rect : public DrawCall
    {
    public:
        Rect(LineInfo &&t_line, color_t t_fill, rect<double> t_rect);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        color_t m_fill;
        rect<double> m_rect;
    };

    class Polyline : public DrawCall
    {
    public:
        Polyline(LineInfo &&t_line, std::vector<vertex<double>> &&t_points);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        std::vector<vertex<double>> m_points;
    };
    class Polygon : public DrawCall
    {
    public:
        Polygon(LineInfo &&t_line, color_t t_fill, std::vector<vertex<double>> &&t_points);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        color_t m_fill;
        std::vector<vertex<double>> m_points;
    };
    class Path : public DrawCall
    {
    public:
        Path(LineInfo &&t_line, color_t t_fill, std::vector<vertex<double>> &&t_points, std::vector<int> &&t_nper, bool t_winding);
        void svg(std::ostream &os) const override;

    private:
        LineInfo m_line;
        color_t m_fill;
        std::vector<vertex<double>> m_points;
        std::vector<int> m_nper;
        bool m_winding;
    };

    class Raster : public DrawCall
    {
    public:
        Raster(std::vector<unsigned int> &&t_raster, vertex<int> t_wh,
               rect<double> t_rect,
               double t_rot,
               bool t_interpolate);
        void svg(std::ostream &os) const override;

    private:
        std::vector<unsigned int> m_raster;
        vertex<int> m_wh;
        rect<double> m_rect;
        double m_rot;
        bool m_interpolate;
    };

    class Clip
    {
    public:
        Clip(clip_id_t t_id, rect<double> t_rect);
        [[nodiscard]] bool equals(rect<double> t_rect) const;
        void svg_def(std::ostream &os) const;
        [[nodiscard]] clip_id_t id() const;

    private:
        clip_id_t m_id;
        rect<double> m_rect;
    };

    class Page
    {
    public:
        Page(page_id_t t_id, vertex<double> t_size);
        void put(std::shared_ptr<DrawCall> t_dc);
        void clear();
        void svg(std::ostream &os, boost::optional<const std::string &> t_extra_css) const;
        void clip(rect<double> t_rect);
        [[nodiscard]] vertex<double> size() const;
        void size(vertex<double> t_size);
        void fill(color_t t_fill);
        [[nodiscard]] page_id_t id() const;

    private:
        page_id_t m_id;
        vertex<double> m_size;
        color_t m_fill;

        std::vector<std::shared_ptr<DrawCall>> m_dcs;
        std::vector<Clip> m_cps;
    };

} // namespace httpgd::dc

#endif /* HTTPGD_DRAWDATA_H */