#ifndef HTTPGD_GEOM_H
#define HTTPGD_GEOM_H

#include <algorithm>

namespace httpgd {
    
    using color_t = int;

    template <class T>
    struct vertex
    {
        T x, y;
    };
    
    template <class T>
    struct rect
    {
        T x, y, width, height;
    };

    template <class T>
    rect<T> normalize_rect(T x0, T y0, T x1, T y1)
    {
        return {
            std::min(x0, x1),
            std::min(y0, y1),
            std::abs(x0 - x1),
            std::abs(y0 - y1)
        };
    }

}

#endif