//
//  Collection of functions for font metrics and raster image encoding.
//  Extrected from: https://github.com/r-lib/svglite/blob/master/src/devSVG.cpp (2020-06-21T15:26:43+00:00)
//
//
//  (C) 2002 T Jake Luciani: SVG device, based on PicTex device
//  (C) 2008 Tony Plate: Line type support from RSVGTipsDevice package
//  (C) 2012 Matthieu Decorde: UTF-8 support, XML reserved characters and XML header
//  (C) 2015 RStudio (Hadley Wickham): modernisation & refactoring
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef HTTPGD_SVGLITE_UTILS_H
#define HTTPGD_SVGLITE_UTILS_H

#include <cpp11/as.hpp>
#include <cpp11/list.hpp>
#include <cstring>
#include <systemfonts.h>
#include <fmt/format.h>




#include <string>

namespace httpgd
{
    // Font handling

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

    inline std::string find_alias_field(std::string family, cpp11::list &alias,
                                        const char *face, const char *field)
    {
        if (alias[face] != R_NilValue)
        {
            cpp11::list font(alias[face]);
            if (font[field] != R_NilValue)
                return cpp11::as_cpp<std::string>(font[field]);
        }
        return std::string();
    }

    inline std::string find_user_alias(std::string family,
                                       cpp11::list const &aliases,
                                       int face, const char *field)
    {
        std::string out;
        if (aliases[family.c_str()] != R_NilValue)
        {
            cpp11::list alias(aliases[family.c_str()]);
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

    inline std::string fontfile(const char *family_, int face,
                                cpp11::list user_aliases)
    {
        std::string family(family_);
        if (face == 5)
            family = "symbol";
        else if (family == "")
            family = "sans";

        return find_user_alias(family, user_aliases, face, "file");
    }

    inline FontSettings get_font_file(const char *family, int face, cpp11::list user_aliases)
    {
        const char *fontfamily = family;
        if (is_symbol(face))
        {
            fontfamily = "symbol";
        }
        else if (strcmp(family, "") == 0)
        {
            fontfamily = "sans";
        }
        std::string alias = fontfile(family, face, user_aliases);
        if (alias.size() > 0)
        {
            FontSettings result = {};
            std::strncpy(result.file, alias.c_str(), PATH_MAX);
            result.index = 0;
            result.n_features = 0;
            return result;
        }

        return locate_font_with_features(fontfamily, is_italic(face), is_bold(face));
    }

    inline std::string find_system_alias(std::string family,
                                         cpp11::list const &aliases)
    {
        std::string out;
        if (aliases[family.c_str()] != R_NilValue)
        {
            cpp11::sexp alias = aliases[family.c_str()];
            if (TYPEOF(alias) == STRSXP && Rf_length(alias) == 1)
                out = cpp11::as_cpp<std::string>(alias);
        }
        return out;
    }

    inline std::string fontname(const char *family_, int face,
                                cpp11::list const &system_aliases,
                                cpp11::list const &user_aliases, 
                                FontSettings &font)
    {
        std::string family(family_);
        if (face == 5)
            family = "symbol";
        else if (family == "")
            family = "sans";

        std::string alias = find_system_alias(family, system_aliases);
        if (!alias.size())
        {
            alias = find_user_alias(family, user_aliases, face, "name");
        }

        if (alias.size())
        {
            return alias;
        }

        const size_t MAX_FONT_FAMILY_LEN = 100;
        char family_name[MAX_FONT_FAMILY_LEN];
        if (get_font_family(font.file, font.index, family_name, MAX_FONT_FAMILY_LEN))
        {
            return std::string(family_name, strnlen(family_name, MAX_FONT_FAMILY_LEN));
        }
        return family;
    }


} // namespace httpgd

#endif