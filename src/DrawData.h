#ifndef HTTPGD_DRAWDATA_H
#define HTTPGD_DRAWDATA_H

#include <string>
#include <vector>
#include <memory>

// Do not include any R headers here !

namespace httpgd
{
    namespace dc
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

        struct TextInfo 
        {
            std::string font_family;
            double fontsize;
            bool bold;
            bool italic;
            double txtwidth_px;
        };

        class Clip;

        class DrawCall
        {
        public:
            int m_col;
            int m_fill;
            double m_gamma;
            double m_lwd;
            int m_lty;
            GC_lineend m_lend;
            GC_linejoin m_ljoin;
            double m_lmitre;
            int m_clip_id;

            explicit DrawCall(const void *gc);
            virtual ~DrawCall();

            virtual void build_svg(std::string *buf) const;
            virtual void build_svg_style(std::string *buf, bool include_fill) const;
        };

        class Text : public DrawCall
        {
        public:
            Text(const void *gc, double x, double y, const std::string &str, double rot, double /*hadj*/, const TextInfo &t_text);
            void build_svg(std::string *buf) const;

        private:
            double m_x, m_y, m_rot/*, m_hadj*/;
            std::string m_str;
            TextInfo m_text;
        };

        class Circle : public DrawCall
        {
        public:
            Circle(const void *gc, double x, double y, double r);
            void build_svg(std::string *buf) const;

        private:
            double m_x, m_y, m_r;
        };

        class Line : public DrawCall
        {
        public:
            Line(const void *gc, double x1, double y1, double x2, double y2);
            void build_svg(std::string *buf) const;

        private:
            double m_x1, m_y1, m_x2, m_y2;
        };

        class Rect : public DrawCall
        {
        public:
            Rect(const void *gc, double x0, double y0, double x1, double y1);
            void build_svg(std::string *buf) const;

        private:
            double m_x0, m_y0, m_x1, m_y1;
        };

        class Polyline : public DrawCall
        {
        public:
            Polyline(const void *gc, int n, const std::vector<double> &x, const std::vector<double> &y);
            void build_svg(std::string *buf) const;

        private:
            int m_n;
            std::vector<double> m_x;
            std::vector<double> m_y;
        };
        class Polygon : public DrawCall
        {
        public:
            Polygon(const void *gc, int n, const std::vector<double> &x, const std::vector<double> &y);
            void build_svg(std::string *buf) const;

        private:
            int m_n;
            std::vector<double> m_x;
            std::vector<double> m_y;
        };
        class Path : public DrawCall
        {
        public:
            Path(const void *gc, const std::vector<double> &x, const std::vector<double> &y, int npoly, const std::vector<int> &nper, bool winding);
            void build_svg(std::string *buf) const;

        private:
            std::vector<double> m_x;
            std::vector<double> m_y;
            int m_npoly;
            std::vector<int> m_nper;
            bool m_winding;
        };

        class Raster : public DrawCall
        {
        public:
            Raster(const void *gc, const std::vector<unsigned int> &raster, int w, int h,
                   double x, double y,
                   double width, double height,
                   double rot,
                   bool interpolate);
            void build_svg(std::string *buf) const;

        private:
            std::vector<unsigned int> m_raster;
            int m_w;
            int m_h;
            double m_x;
            double m_y;
            double m_width;
            double m_height;
            double m_rot;
            bool m_interpolate;
        };

        class Clip
        {
        public:
            Clip(int id, double x0, double x1, double y0, double y1);
            bool equals(double x0, double x1, double y0, double y1);
            void build_svg_def(std::string *buf) const;
            static void build_svg_attr(std::string *buf, int id);
            int id() const;

        protected:
            int m_id;
            double m_x0;
            double m_x1;
            double m_y0;
            double m_y1;
        };

        class Page
        {
        public:
            double width;
            double height;
            int fill;

            Page(double t_width, double t_height);
            void put(std::shared_ptr<DrawCall> dc);
            void clear();
            void build_svg(std::string *buf) const;
            void clip(double x0, double x1, double y0, double y1);

        private:
            std::vector<std::shared_ptr<DrawCall>> m_dcs;
            std::vector<Clip> m_cps;
        };

    } // namespace dc
} // namespace httpgd

#endif /* HTTPGD_DRAWDATA_H */