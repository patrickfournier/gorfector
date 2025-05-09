#include "ZooLib/StringUtils.hpp"
#include "gtest/gtest.h"

namespace ZooLib
{
    TEST(ZooLib_StringUtilsTests, TrimLeftWorks)
    {
        std::string str = "   Hello, World!";
        LTrim(str);
        EXPECT_EQ("Hello, World!", str);

        str = "   Hello, World!   ";
        LTrim(str);
        EXPECT_EQ("Hello, World!   ", str);

        str = "   ";
        LTrim(str);
        EXPECT_EQ("", str);
    }

    TEST(ZooLib_StringUtilsTests, TrimRightWorks)
    {
        std::string str = "Hello, World!   ";
        RTrim(str);
        EXPECT_EQ("Hello, World!", str);

        str = "   Hello, World!   ";
        RTrim(str);
        EXPECT_EQ("   Hello, World!", str);

        str = "   ";
        RTrim(str);
        EXPECT_EQ("", str);
    }

    TEST(ZooLib_StringUtilsTests, TrimWorks)
    {
        std::string str = "   Hello, World!   ";
        Trim(str);
        EXPECT_EQ("Hello, World!", str);

        str = "Hello, World!";
        Trim(str);
        EXPECT_EQ("Hello, World!", str);

        str = "   ";
        Trim(str);
        EXPECT_EQ("", str);
    }
}
