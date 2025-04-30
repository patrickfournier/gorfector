#pragma once

#include "Point.hpp"

namespace Gorfector
{
    template<typename T>
    struct Rect
    {
        T x{};
        T y{};
        T width{};
        T height{};

        T MinX() const
        {
            if (width < 0)
            {
                return x + width;
            }
            return x;
        }
        T MinY() const
        {
            if (height < 0)
            {
                return y + height;
            }
            return y;
        }
        T MaxX() const
        {
            if (width < 0)
            {
                return x;
            }
            return x + width;
        }
        T MaxY() const
        {
            if (height < 0)
            {
                return y;
            }
            return y + height;
        }

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
