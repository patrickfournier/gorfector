#pragma once

#include <algorithm>
#include <string>

namespace ZooLib
{
    /**
     * \brief Removes leading whitespace characters from the string (in place).
     * \param s The string to be trimmed.
     */
    inline void LTrim(std::string &s)
    {
        s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) { return !std::isspace(ch); }));
    }

    /**
     * \brief Removes trailing whitespace characters from the string (in place).
     * \param s The string to be trimmed.
     */
    inline void RTrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) { return !std::isspace(ch); }).base(),
                s.end());
    }

    /**
     * \brief Removes both leading and trailing whitespace characters from the string (in place).
     * \param s The string to be trimmed.
     */
    inline void Trim(std::string &s)
    {
        RTrim(s);
        LTrim(s);
    }
}
