#pragma once

namespace Gorfector
{
    template<typename T>
    struct Point
    {
        T x{};
        T y{};

        bool operator==(const Point &point) const
        {
            return x == point.x && y == point.y;
        }

        Point &operator+=(const Point &point)
        {
            x += point.x;
            y += point.y;
            return *this;
        }

        friend Point operator+(Point lhs, const Point &rhs)
        {
            lhs += rhs;
            return lhs;
        }
    };
}
