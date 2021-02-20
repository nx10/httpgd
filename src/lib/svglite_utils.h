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

#define R_NO_REMAP
#include <R_ext/GraphicsEngine.h>

extern "C"
{
#include <png.h>
}

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

    // Raster image encoding

    const static char encode_lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const static char pad_character = '=';
    inline std::string base64_encode(const std::uint8_t *buffer, size_t size)
    {
        std::string encoded_string;
        encoded_string.reserve(((size / 3) + (size % 3 > 0)) * 4);
        std::uint32_t temp{};
        int index = 0;
        for (size_t idx = 0; idx < size / 3; idx++)
        {
            temp = buffer[index++] << 16; //Convert to big endian
            temp += buffer[index++] << 8;
            temp += buffer[index++];
            encoded_string.append(1, encode_lookup[(temp & 0x00FC0000) >> 18]);
            encoded_string.append(1, encode_lookup[(temp & 0x0003F000) >> 12]);
            encoded_string.append(1, encode_lookup[(temp & 0x00000FC0) >> 6]);
            encoded_string.append(1, encode_lookup[(temp & 0x0000003F)]);
        }
        switch (size % 3)
        {
        case 1:
            temp = buffer[index++] << 16; //Convert to big endian
            encoded_string.append(1, encode_lookup[(temp & 0x00FC0000) >> 18]);
            encoded_string.append(1, encode_lookup[(temp & 0x0003F000) >> 12]);
            encoded_string.append(2, pad_character);
            break;
        case 2:
            temp = buffer[index++] << 16; //Convert to big endian
            temp += buffer[index++] << 8;
            encoded_string.append(1, encode_lookup[(temp & 0x00FC0000) >> 18]);
            encoded_string.append(1, encode_lookup[(temp & 0x0003F000) >> 12]);
            encoded_string.append(1, encode_lookup[(temp & 0x00000FC0) >> 6]);
            encoded_string.append(1, pad_character);
            break;
        }
        return encoded_string;
    }

    static void png_memory_write(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        std::vector<uint8_t> *p = (std::vector<uint8_t> *)png_get_io_ptr(png_ptr);
        p->insert(p->end(), data, data + length);
    }
    inline std::string raster_to_string(std::vector<unsigned int> raster_, int w, int h, double width, double height, bool interpolate)
    {
        unsigned int *raster = raster_.data();

        h = h < 0 ? -h : h;
        w = w < 0 ? -w : w;
        bool resize = false;
        int w_fac = 1, h_fac = 1;
        std::vector<unsigned int> raster_resize;

        if (!interpolate && double(w) < width)
        {
            resize = true;
            w_fac = std::ceil(width / w);
        }
        if (!interpolate && double(h) < height)
        {
            resize = true;
            h_fac = std::ceil(height / h);
        }

        if (resize)
        {
            int w_new = w * w_fac;
            int h_new = h * h_fac;
            raster_resize.reserve(w_new * h_new);
            for (int i = 0; i < h; ++i)
            {
                for (int j = 0; j < w; ++j)
                {
                    unsigned int val = raster[i * w + j];
                    for (int wrep = 0; wrep < w_fac; ++wrep)
                    {
                        raster_resize.push_back(val);
                    }
                }
                for (int hrep = 1; hrep < h_fac; ++hrep)
                {
                    raster_resize.insert(raster_resize.end(), raster_resize.end() - w_new, raster_resize.end());
                }
            }
            raster = raster_resize.data();
            w = w_new;
            h = h_new;
        }

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png)
        {
            return "";
        }
        png_infop info = png_create_info_struct(png);
        if (!info)
        {
            png_destroy_write_struct(&png, (png_infopp)NULL);
            return "";
        }
        if (setjmp(png_jmpbuf(png)))
        {
            png_destroy_write_struct(&png, &info);
            return "";
        }
        png_set_IHDR(
            png,
            info,
            w, h,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);
        std::vector<uint8_t *> rows(h);
        for (int y = 0; y < h; ++y)
        {
            rows[y] = (uint8_t *)raster + y * w * 4;
        }

        std::vector<std::uint8_t> buffer;
        png_set_rows(png, info, &rows[0]);
        png_set_write_fn(png, &buffer, png_memory_write, NULL);
        png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
        png_destroy_write_struct(&png, &info);

        return base64_encode(buffer.data(), buffer.size());
    }

    inline void write_xml_escaped(fmt::memory_buffer &os, const std::string &text)
    {
        for (const char &c : text)
        {
            switch (c)
            {
            case '&':
                fmt::format_to(os, "&amp;");
                break;
            case '<':
                fmt::format_to(os, "&lt;");
                break;
            case '>':
                fmt::format_to(os, "&gt;");
                break;
            case '"':
                fmt::format_to(os, "&quot;");
                break;
            case '\'':
                fmt::format_to(os, "&apos;");
                break;
            default:
                fmt::format_to(os, "{}", c);
            }
        }
    }

} // namespace httpgd

#endif