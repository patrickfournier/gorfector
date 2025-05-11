#pragma once

#include <stdexcept>

namespace Gorfector
{
    /**
     * \class SaneException
     * \brief Exception class for handling SANE-related errors.
     *
     * This class extends `std::runtime_error` to provide a specific exception
     * type for errors encountered when interacting with SANE devices.
     */
    class SaneException : public std::runtime_error
    {
    public:
        /**
         * \brief Constructs a SaneException with a given error message.
         * \param arg The error message describing the exception.
         */
        explicit SaneException(const std::string &arg)
            : runtime_error(arg)
        {
        }
    };
}
