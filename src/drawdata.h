#ifndef HTTPGD_DRAWDATA_H
#define HTTPGD_DRAWDATA_H

//#include <iostream>
#include <string>
#include <vector>
//#include <algorithm>


namespace httpgd
{
  namespace dc
  {
    typedef enum
    {
      GC_ROUND_CAP = 1,
      GC_BUTT_CAP = 2,
      GC_SQUARE_CAP = 3
    } GC_lineend;

    typedef enum
    {
      GC_ROUND_JOIN = 1,
      GC_MITRE_JOIN = 2,
      GC_BEVEL_JOIN = 3
    } GC_linejoin;

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
      Clip *m_clip;

      DrawCall();
      virtual ~DrawCall();

      virtual void to_svg(std::string &buf);
    };

    class Text : public DrawCall
    {
    public:
      Text(double x, double y, const std::string &str, double rot, double hadj);
      void to_svg(std::string &buf);

      std::string m_font_family;
      double m_fontsize;
      bool m_bold;
      bool m_italic;
      double m_txtwidth_px;

    private:
      double m_x, m_y, m_rot, m_hadj;
      std::string m_str;
    };

    class Circle : public DrawCall
    {
    public:
      Circle(double x, double y, double r);
      void to_svg(std::string &buf);

    private:
      double m_x, m_y, m_r;
    };

    class Line : public DrawCall
    {
    public:
      Line(double x1, double y1, double x2, double y2);
      void to_svg(std::string &buf);

    private:
      double m_x1, m_y1, m_x2, m_y2;
    };

    class Rect : public DrawCall
    {
    public:
      Rect(double x0, double y0, double x1, double y1);
      void to_svg(std::string &buf);

    private:
      double m_x0, m_y0, m_x1, m_y1;
    };

    class Polyline : public DrawCall
    {
    public:
      Polyline(int n, std::vector<double> &x, std::vector<double> &y);
      void to_svg(std::string &buf);

    private:
      int m_n;
      std::vector<double> m_x;
      std::vector<double> m_y;
    };
    class Polygon : public DrawCall
    {
    public:
      Polygon(int n, std::vector<double> &x, std::vector<double> &y);
      void to_svg(std::string &buf);

    private:
      int m_n;
      std::vector<double> m_x;
      std::vector<double> m_y;
    };
    class Path : public DrawCall
    {
    public:
      Path(std::vector<double> &x, std::vector<double> &y, int npoly, std::vector<int> &nper, bool winding);
      void to_svg(std::string &buf);

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
      Raster(std::vector<unsigned int> &raster, int w, int h,
             double x, double y,
             double width, double height,
             double rot,
             bool interpolate);
      void to_svg(std::string &buf);

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
      void to_svg_def(std::string &buf);
      void to_svg_attr(std::string &buf);

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
      double m_width;
      double m_height;
      int m_fill;
      Page(double width, double height);
      ~Page();
      void put(DrawCall *dc);
      void clear();
      void to_svg(std::string &buf);
      void clip(double x0, double x1, double y0, double y1);
      int get_upid();

    private:
      std::vector<DrawCall *> m_dcs;
      std::vector<Clip> m_cps;
      int m_upid;
    };

  } // namespace dc
} // namespace httpgd

#endif /* HTTPGD_DRAWDATA_H */