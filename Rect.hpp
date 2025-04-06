#pragma once

namespace ZooScan
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
    };

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
    };
}
