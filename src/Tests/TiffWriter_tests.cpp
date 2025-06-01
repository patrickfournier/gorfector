#include "gtest/gtest.h"

#include "CompareFiles.hpp"
#include "Writers/TiffWriter.hpp"

#include "ImageGenerator.hpp"

namespace Gorfector
{
    class Gorfector_TiffWriterTestsFixture : public testing::Test
    {
    protected:
        ZooLib::State *m_State{};
        std::filesystem::path m_TestFilePath{};
        std::filesystem::path m_ExpectedFilePath{};

        void SetUp() override
        {
            const testing::TestInfo *const testInfo = testing::UnitTest::GetInstance()->current_test_info();

            m_State = new ZooLib::State();
            std::string fileName{};
            if (testInfo != nullptr)
            {
                fileName = std::string(testInfo->test_suite_name()) + "_" + std::string(testInfo->name()) + ".tiff";
            }
            m_TestFilePath = std::filesystem::path(testing::TempDir()) / fileName;

            std::string expectedFileName{};
            if (testInfo != nullptr)
            {
                expectedFileName = std::string(testInfo->test_suite_name()) + "_" + std::string(testInfo->name()) +
                                   "_expected.tiff";
            }
            m_ExpectedFilePath = std::filesystem::path(g_DataDir) / expectedFileName;
        }

        void TearDown() override
        {
            delete m_State;

            if (g_CleanArtifacts && std::filesystem::exists(m_TestFilePath))
            {
                std::filesystem::remove(m_TestFilePath);
            }
        }
    };

    TEST_F(Gorfector_TiffWriterTestsFixture, CanCreateTiffWriter)
    {
        SANE_Parameters saneParameters{
                .format = SANE_FRAME_RGB,
                .last_frame = SANE_FALSE,
                .bytes_per_line = 300,
                .pixels_per_line = 100,
                .lines = 100,
                .depth = 8,
        };

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        writer.CancelFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
    }

    TEST_F(Gorfector_TiffWriterTestsFixture, CanWrite1bitTiff)
    {
        SANE_Parameters saneParameters;
        SANE_Byte *buffer = nullptr;
        size_t bufferSize = 0;
        ImageGenerator::Generate1BitImage(100, 100, &saneParameters, &buffer, &bufferSize);

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        for (auto i = 0; i < saneParameters.lines; ++i)
        {
            auto row = buffer + i * saneParameters.bytes_per_line;
            auto byteWritten = writer.AppendBytes(row, 1, saneParameters);
            EXPECT_EQ(byteWritten, saneParameters.bytes_per_line) << "Failed to write row " << i;
        }
        writer.CloseFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
        ASSERT_FILE_EQ(m_TestFilePath, m_ExpectedFilePath, "");

        delete[] buffer;
    }

    TEST_F(Gorfector_TiffWriterTestsFixture, CanWrite8BitColorTiff)
    {
        SANE_Parameters saneParameters;
        SANE_Byte *buffer = nullptr;
        size_t bufferSize = 0;
        ImageGenerator::Generate8BitColorImage(100, 100, &saneParameters, &buffer, &bufferSize);

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        for (auto i = 0; i < saneParameters.lines; ++i)
        {
            auto row = buffer + i * saneParameters.bytes_per_line;
            auto byteWritten = writer.AppendBytes(row, 1, saneParameters);
            EXPECT_EQ(byteWritten, saneParameters.bytes_per_line) << "Failed to write row " << i;
        }
        writer.CloseFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
        ASSERT_FILE_EQ(m_TestFilePath, m_ExpectedFilePath, "");

        delete[] buffer;
    }

    TEST_F(Gorfector_TiffWriterTestsFixture, CanWrite8BitGrayscaleTiff)
    {
        SANE_Parameters saneParameters;
        SANE_Byte *buffer = nullptr;
        size_t bufferSize = 0;
        ImageGenerator::Generate8BitGrayscaleImage(100, 100, &saneParameters, &buffer, &bufferSize);

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        for (auto i = 0; i < saneParameters.lines; ++i)
        {
            auto row = buffer + i * saneParameters.bytes_per_line;
            auto byteWritten = writer.AppendBytes(row, 1, saneParameters);
            EXPECT_EQ(byteWritten, saneParameters.bytes_per_line) << "Failed to write row " << i;
        }
        writer.CloseFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
        ASSERT_FILE_EQ(m_TestFilePath, m_ExpectedFilePath, "");

        delete[] buffer;
    }

    TEST_F(Gorfector_TiffWriterTestsFixture, CanWrite16bitsColorTiff)
    {
        SANE_Parameters saneParameters;
        SANE_Byte *buffer = nullptr;
        size_t bufferSize = 0;
        ImageGenerator::Generate16BitColorImage(100, 100, &saneParameters, &buffer, &bufferSize);

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        for (auto i = 0; i < saneParameters.lines; ++i)
        {
            auto row = buffer + i * saneParameters.bytes_per_line;
            auto byteWritten = writer.AppendBytes(row, 1, saneParameters);
            EXPECT_EQ(byteWritten, saneParameters.bytes_per_line) << "Failed to write row " << i;
        }
        writer.CloseFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
        ASSERT_FILE_EQ(m_TestFilePath, m_ExpectedFilePath, "");

        delete[] buffer;
    }

    TEST_F(Gorfector_TiffWriterTestsFixture, CanWrite16bitsGrayscaleTiff)
    {
        SANE_Parameters saneParameters;
        SANE_Byte *buffer = nullptr;
        size_t bufferSize = 0;
        ImageGenerator::Generate16BitGrayscaleImage(100, 100, &saneParameters, &buffer, &bufferSize);

        TiffWriter writer(m_State, std::string(typeid(Gorfector_TiffWriterTestsFixture).name()));
        writer.CreateFile(m_TestFilePath, nullptr, saneParameters);
        for (auto i = 0; i < saneParameters.lines; ++i)
        {
            auto row = buffer + i * saneParameters.bytes_per_line;
            auto byteWritten = writer.AppendBytes(row, 1, saneParameters);
            EXPECT_EQ(byteWritten, saneParameters.bytes_per_line) << "Failed to write row " << i;
        }
        writer.CloseFile();

        EXPECT_TRUE(std::filesystem::exists(m_TestFilePath));
        ASSERT_FILE_EQ(m_TestFilePath, m_ExpectedFilePath, "");

        delete[] buffer;
    }
}
