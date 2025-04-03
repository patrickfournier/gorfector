#pragma once

#include <stdexcept>

namespace ZooScan
{
    class SaneException : public std::runtime_error
    {
    public:
        explicit SaneException(const std::string &arg)
            : runtime_error(arg)
        {
        }
    };
}
