#pragma once

#include <stdexcept>

namespace Gorfector
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
