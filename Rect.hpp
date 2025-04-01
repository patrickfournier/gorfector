#pragma once

namespace ZooScan
{
    template<typename T>
    struct Rect
    {
        T x{};
        T y{};
        T width{};
        T height{};
    };
}
