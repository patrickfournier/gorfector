#include "CompareFiles.hpp"

#include <filesystem>
#include <format>
#include <fstream>

#include "gtest/gtest.h"

extern std::string g_DataDir;

bool ASSERT_FILE_EQ(
        const std::string &testFile, const std::string &expected, const std::string &diffParams,
        std::source_location local)
{
    const auto msg = std::format("ASSERT_FILE_EQ at: {}:{}", local.file_name(), local.line());
    SCOPED_TRACE(msg);
    EXPECT_TRUE(std::filesystem::exists(testFile)) << testFile + " does not exist";
    EXPECT_TRUE(std::filesystem::exists(expected)) << expected + " does not exist";

    if (std::filesystem::exists(testFile) && std::filesystem::exists(expected))
    {
        if (system(("diff " + testFile + " " + expected + " " + diffParams + " > result.diff").c_str()) == 0)
        {
            std::filesystem::remove("result.diff");
            return true;
        }

        ADD_FAILURE() << "diff " + testFile + " " + expected;
        std::ifstream fp("result.diff");
        if (fp.is_open() == false)
        {
            ADD_FAILURE() << "cannot open diff file result.diff";
        }
        else
        {
            std::cout << std::endl;
            std::cout << std::format("============ start diff between [{}] and [{}] ===========\n", testFile, expected);
            std::string line;
            while (getline(fp, line))
            {
                std::cout << line << std::endl;
            }
            std::cout << std::format("============= end diff between [{}] and [{}] ============\n", testFile, expected);
            fp.close();
        }

        std::filesystem::remove("result.diff");
    }

    return false;
}
