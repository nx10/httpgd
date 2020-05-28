// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>
#include <gdtools.h>
#include <string>

#include "FontAnalyzer.h"

// Most of this is copied from svglite

namespace httpgd
{

    inline bool is_black(int col)
    {
        return (R_RED(col) == 0) && (R_GREEN(col) == 0) && (R_BLUE(col) == 0) &&
               (R_ALPHA(col) == 255);
    }

    inline bool is_filled(int col)
    {
        return R_ALPHA(col) != 0;
    }

    inline bool is_bold(int face)
    {
        return face == 2 || face == 4;
    }
    inline bool is_italic(int face)
    {
        return face == 3 || face == 4;
    }
    inline bool is_bolditalic(int face)
    {
        return face == 4;
    }
    inline bool is_symbol(int face)
    {
        return face == 5;
    }

    inline std::string find_alias_field(std::string &family, Rcpp::List &alias,
                                        const char *face, const char *field)
    {
        if (alias.containsElementNamed(face))
        {
            Rcpp::List font = alias[face];
            if (font.containsElementNamed(field)) {
                return font[field];
            }
        }
        return std::string();
    }

    inline std::string find_user_alias(std::string &family,
                                       Rcpp::List const &aliases,
                                       int face, const char *field)
    {
        std::string out;
        if (aliases.containsElementNamed(family.c_str()))
        {
            Rcpp::List alias = aliases[family];
            if (is_bolditalic(face))
                out = find_alias_field(family, alias, "bolditalic", field);
            else if (is_bold(face))
                out = find_alias_field(family, alias, "bold", field);
            else if (is_italic(face))
                out = find_alias_field(family, alias, "italic", field);
            else if (is_symbol(face))
                out = find_alias_field(family, alias, "symbol", field);
            else
                out = find_alias_field(family, alias, "plain", field);
        }
        return out;
    }

    inline std::string find_system_alias(std::string &family,
                                         Rcpp::List const &aliases)
    {
        std::string out;
        if (aliases.containsElementNamed(family.c_str()))
        {
            SEXP alias = aliases[family];
            if (TYPEOF(alias) == STRSXP && Rf_length(alias) == 1)
                out = Rcpp::as<std::string>(alias);
        }
        return out;
    }

    inline std::string fontname(const char *family_, int face,
                                Rcpp::List const &system_aliases,
                                Rcpp::List const &user_aliases)
    {
        std::string family(family_);
        if (face == 5)
            family = "symbol";
        else if (family == "")
            family = "sans";

        std::string alias = find_system_alias(family, system_aliases);
        if (!alias.size())
            alias = find_user_alias(family, user_aliases, face, "name");

        if (alias.size())
            return alias;
        else
            return family;
    }

    inline std::string fontfile(const char *family_, int face,
                                Rcpp::List user_aliases)
    {
        std::string family(family_);
        if (face == 5)
            family = "symbol";
        else if (family == "")
            family = "sans";

        return find_user_alias(family, user_aliases, face, "file");
    }

    FontAnalyzer::FontAnalyzer(Rcpp::List t_aliases) : m_system_aliases(Rcpp::wrap(t_aliases["system"])),
                                                       m_user_aliases(Rcpp::wrap(t_aliases["user"])),
                                                       m_cc(gdtools::context_create())
    {
    }

    void FontAnalyzer::analyze(const std::string &str, const pGEcontext gc)
    {
        m_rfontface = gc->fontface;
        m_rfontsize = gc->cex * gc->ps;
        m_font_file = fontfile(gc->fontfamily, gc->fontface, m_user_aliases);
        m_font_name = fontname(gc->fontfamily, gc->fontface, m_system_aliases, m_user_aliases);
        gdtools::context_set_font(m_cc, m_font_name, gc->cex * gc->ps, httpgd::is_bold(gc->fontface), httpgd::is_italic(gc->fontface), m_font_file);
        m_font_metric = gdtools::context_extents(m_cc, str);
    }

    bool FontAnalyzer::is_bold() const { return httpgd::is_bold(m_rfontface); }
    bool FontAnalyzer::is_italic() const { return httpgd::is_italic(m_rfontface); }

    double FontAnalyzer::get_ascent() const { return m_font_metric.ascent; }
    double FontAnalyzer::get_descent() const { return m_font_metric.descent; }
    double FontAnalyzer::get_width() const { return m_font_metric.width; }
    double FontAnalyzer::get_fontsize() const { return m_rfontsize; }
    std::string FontAnalyzer::get_font_family() const { return m_font_name; }

    std::string char_r_unicode(int c)
    {
        bool is_unicode = mbcslocale;
        if (c < 0)
        {
            is_unicode = true;
            c = -c;
        }

        // Convert to string - negative implies unicode code point
        char str[16];
        if (is_unicode)
        {
            Rf_ucstoutf8(str, (unsigned int)c);
        }
        else
        {
            str[0] = (char)c;
            str[1] = '\0';
        }
        return std::string(str);
    }

} // namespace httpgd