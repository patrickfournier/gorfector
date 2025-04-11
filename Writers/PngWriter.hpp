#pragma once

#include <App.hpp>
#include <png.h>
#include <zlib.h>

#include "FileWriter.hpp"
#include "PngWriterStateComponent.hpp"

namespace ZooScan
{

    class PngWriter : public FileWriter
    {
        static const std::vector<std::string> k_Extensions;
        static constexpr std::string k_Name = "PNG";

        PngWriterStateComponent *m_StateComponent{};
        FILE *m_File{};
        png_structp m_Png{};
        png_infop m_PngInfo{};
        SANE_Byte **m_LinePointers{};
        int m_LinePointerSize{};

    public:
        explicit PngWriter(ZooLib::State *state)
        {
            m_StateComponent = new PngWriterStateComponent(state);
        }

        ~PngWriter() override
        {
            delete m_StateComponent;
        }

        [[nodiscard]] const std::string &GetName() const override
        {
            return k_Name;
        }

        [[nodiscard]] std::vector<std::string> GetExtensions() const override
        {
            return k_Extensions;
        }

        Error CreateFile(
                const App &app, std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters, SANE_Byte *image) override
        {
            if (parameters.lines > 0 &&
                static_cast<size_t>(parameters.lines) > PNG_SIZE_MAX / parameters.bytes_per_line)
            {
                return Error::ImageTooLarge;
            }

            m_File = fopen(path.c_str(), "wb");
            if (m_File == nullptr)
            {
                return Error::CannotOpenFile;
            }

            m_Png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (!m_Png)
            {
                return Error::UnknownError;
            }

            if (setjmp(png_jmpbuf(m_Png)))
            {
                return Error::UnknownError;
            }

            m_PngInfo = png_create_info_struct(m_Png);
            if (!m_PngInfo)
            {
                png_destroy_write_struct(&m_Png, nullptr);
                return Error::UnknownError;
            }

            png_init_io(m_Png, m_File);

            png_set_IHDR(
                    m_Png, m_PngInfo, parameters.pixels_per_line, parameters.lines, parameters.depth,
                    parameters.format == SANE_FRAME_RGB ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

            png_set_compression_level(m_Png, m_StateComponent->GetCompressionLevel());

            std::string appId = app.GetApplicationId();
            std::string creator = std::string(deviceOptions->GetDeviceMaker()) + " " + deviceOptions->GetDeviceModel();

            constexpr int numTexts = 2;
            png_text textData[numTexts];
            textData[0].key = const_cast<char *>("Software");
            textData[0].text = const_cast<char *>(appId.c_str());
            textData[0].text_length = strlen(textData[0].text);
            textData[0].compression = PNG_TEXT_COMPRESSION_NONE;
            textData[0].itxt_length = 0;
            textData[0].lang = nullptr;
            textData[0].lang_key = nullptr;

            textData[1].key = const_cast<char *>("Source");
            textData[1].text = const_cast<char *>(creator.c_str());
            textData[1].text_length = strlen(textData[1].text);
            textData[1].compression = PNG_TEXT_COMPRESSION_NONE;
            textData[1].itxt_length = 0;
            textData[1].lang = nullptr;
            textData[1].lang_key = nullptr;

            png_set_text(m_Png, m_PngInfo, textData, numTexts);

            png_write_info(m_Png, m_PngInfo);

            // ReSharper disable once CppRedundantBooleanExpressionArgument
            if (std::endian::native == std::endian::little && parameters.depth > 8)
            {
                png_set_swap(m_Png);
            }

            png_set_flush(m_Png, 128);

            return Error::None;
        }

        int32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, int pixelsPerLine, int bytesPerLine) override
        {
            if (m_LinePointers == nullptr || m_LinePointerSize < numberOfLines)
            {
                if (m_LinePointers != nullptr)
                {
                    delete[] m_LinePointers;
                }

                m_LinePointers = new SANE_Byte *[numberOfLines];
                m_LinePointerSize = numberOfLines;
            }

            int i = 0;

            if (setjmp(png_jmpbuf(m_Png)))
            {
                return i * bytesPerLine;
            }

            for (i = 0; i < numberOfLines; ++i)
            {
                m_LinePointers[i] = bytes + i * bytesPerLine;
            }
            png_write_rows(m_Png, m_LinePointers, m_LinePointerSize);

            return numberOfLines * bytesPerLine;
        }

        void CloseFile() override
        {
            if (setjmp(png_jmpbuf(m_Png)))
            {
                fclose(m_File);

                if (m_LinePointers != nullptr)
                {
                    delete[] m_LinePointers;
                    m_LinePointers = nullptr;
                }

                m_File = nullptr;
                m_Png = nullptr;
                m_PngInfo = nullptr;
                return;
            }

            png_write_end(m_Png, nullptr);
            png_destroy_write_struct(&m_Png, &m_PngInfo);
            fclose(m_File);

            if (m_LinePointers != nullptr)
            {
                delete[] m_LinePointers;
                m_LinePointers = nullptr;
            }

            m_File = nullptr;
            m_Png = nullptr;
            m_PngInfo = nullptr;
        }

        void CancelFile() override
        {
            png_destroy_write_struct(&m_Png, &m_PngInfo);
            fclose(m_File);

            if (m_LinePointers != nullptr)
            {
                delete[] m_LinePointers;
                m_LinePointers = nullptr;
            }

            m_File = nullptr;
            m_Png = nullptr;
            m_PngInfo = nullptr;
        }
    };

}
