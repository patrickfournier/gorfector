#pragma once

#include "Point.hpp"

namespace ZooScan
{
    template<typename T>
    struct Rect
    {
        T x{};
        T y{};
        T width{};
        T height{};

        bool operator==(const Rect &rect) const
        {
            return x == rect.x && y == rect.y && width == rect.width && height == rect.height;
        }

        Rect &operator-=(const Point<double> &point)
        {
            x -= point.x;
            y -= point.y;
            return *this;
        }

        friend Rect<double> operator-(Rect lhs, const Point<double> &rhs)
        {
            lhs -= rhs;
            return lhs;
        }
    };
}
