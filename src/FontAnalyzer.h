#ifndef HTTPGD_FONT_ANALYZER_H
#define HTTPGD_FONT_ANALYZER_H

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>
#include <gdtools.h>

#include <string>

namespace httpgd
{
    class FontAnalyzer
    {
    public:
        explicit FontAnalyzer(Rcpp::List t_aliases);
        void analyze(const std::string &str, const pGEcontext gc);

        bool is_bold() const;
        bool is_italic() const;

        double get_ascent() const;
        double get_descent() const;
        double get_width() const;
        double get_fontsize() const;
        
        std::string get_font_family() const;

    private:
        Rcpp::List m_system_aliases;
        Rcpp::List m_user_aliases;
        XPtrCairoContext m_cc;

        std::string m_font_file;
        std::string m_font_name;
        FontMetric m_font_metric;
        double m_rfontsize;
        int m_rfontface;
    };

    std::string char_r_unicode(int c);
} // namespace httpgd

#endif