#pragma once

#include <source_location>
#include <string>

extern std::string g_DataDir; // Directory where test data files are located
extern bool g_CleanArtifacts; // Whether to clean up test artifacts after tests

bool ASSERT_FILE_EQ(
        const std::string &testFile, const std::string &expected, const std::string &diffParams,
        std::source_location local = std::source_location::current());
