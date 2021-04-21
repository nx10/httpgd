#ifndef HTTPGD_GEOM_H
#define HTTPGD_GEOM_H

#include <algorithm>

namespace httpgd {
    
    using color_t = int;

    template <class T>
    struct gvertex
    {
        T x, y;
    };
    
    template <class T>
    struct grect
    {
        T x, y, width, height;
    };

    template <class T>
    grect<T> normalize_rect(T x0, T y0, T x1, T y1)
    {
        return {
            std::min(x0, x1),
            std::min(y0, y1),
            std::abs(x0 - x1),
            std::abs(y0 - y1)
        };
    }

    template <class T>
    bool rect_equals(grect<T> r0, grect<T> r1, T eps)
    {
        return (std::abs(r0.x - r1.x) < eps) &&
               (std::abs(r0.x - r1.x) < eps) &&
               (std::abs(r0.width - r1.width) < eps) &&
               (std::abs(r0.height - r1.height) < eps);
    }

} // namespace httpgd

#endif