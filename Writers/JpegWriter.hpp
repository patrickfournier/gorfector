#pragma once

#include <string>
#include <vector>

#include <jpeglib.h>

#include "FileWriter.hpp"
#include "JpegWriterState.hpp"

namespace ZooScan
{
    class JpegWriter : public FileWriter
    {
        static const std::vector<std::string> k_Extensions;
        static constexpr std::string k_Name = "JPEG";

        JpegWriterState *m_StateComponent{};

        jpeg_compress_struct *m_CompressStruct{};
        jpeg_error_mgr *m_ErrorHandler{};
        FILE *m_File{};

    public:
        explicit JpegWriter(ZooLib::State *state)
        {
            m_StateComponent = new JpegWriterState(state);
        }

        ~JpegWriter()
        {
            delete m_StateComponent;
        }

        [[nodiscard]] JpegWriterState *GetStateComponent()
        {
            return m_StateComponent;
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
            m_CompressStruct = new jpeg_compress_struct();
            m_ErrorHandler = new jpeg_error_mgr();

            m_CompressStruct->err = jpeg_std_error(m_ErrorHandler);
            jpeg_create_compress(m_CompressStruct);
            m_File = fopen(path.string().c_str(), "wb");
            if (m_File == nullptr)
            {
                jpeg_destroy_compress(m_CompressStruct);
                delete m_CompressStruct;
                delete m_ErrorHandler;
                return Error::CannotOpenFile;
            }

            jpeg_stdio_dest(m_CompressStruct, m_File);
            m_CompressStruct->image_width = parameters.pixels_per_line;
            m_CompressStruct->image_height = parameters.lines;
            m_CompressStruct->input_components = parameters.format == SANE_FRAME_RGB ? 3 : 1;
            m_CompressStruct->in_color_space = parameters.format == SANE_FRAME_RGB ? JCS_RGB : JCS_GRAYSCALE;
            jpeg_set_defaults(m_CompressStruct);
            jpeg_set_quality(m_CompressStruct, m_StateComponent->GetQuality(), TRUE);
            jpeg_start_compress(m_CompressStruct, TRUE);

            return Error::None;
        }

        int32_t
        AppendBytes(SANE_Byte *bytes, int numberOfLines, int pixelsPerLine, int bytesPerLine, int bitDepth) override
        {
            if (m_CompressStruct->image_height <= m_CompressStruct->next_scanline)
                return 0;

            auto numLinesToWrite = std::min(
                    static_cast<unsigned int>(numberOfLines),
                    m_CompressStruct->image_height - m_CompressStruct->next_scanline);

            auto rowPointer = new JSAMPROW[numLinesToWrite];

            if (bitDepth == 8)
            {
                for (auto i = 0U; i < numLinesToWrite; ++i)
                {
                    rowPointer[i] = &bytes[i * bytesPerLine];
                }
            }
            else if (bitDepth == 16)
            {
                constexpr auto offset = std::endian::native == std::endian::little ? 1 : 0;

                for (auto j = 0U; j < numLinesToWrite; ++j)
                {
                    auto line = &bytes[j * bytesPerLine];
                    for (auto i = 0; i < pixelsPerLine; ++i)
                    {
                        bytes[3 * (j * pixelsPerLine + i)] = line[6 * i + offset];
                        bytes[3 * (j * pixelsPerLine + i) + 1] = line[6 * i + 2 + offset];
                        bytes[3 * (j * pixelsPerLine + i) + 2] = line[6 * i + 4 + offset];
                    }

                    rowPointer[j] = &bytes[3 * j * pixelsPerLine];
                }
            }

            jpeg_write_scanlines(m_CompressStruct, rowPointer, numLinesToWrite);

            delete[] rowPointer;
            return numLinesToWrite * bytesPerLine;
        }

        void CloseFile() override
        {
            jpeg_finish_compress(m_CompressStruct);
            fclose(m_File);
            jpeg_destroy_compress(m_CompressStruct);
            delete m_CompressStruct;
            delete m_ErrorHandler;
        }

        void CancelFile() override
        {
            fclose(m_File);
            jpeg_destroy_compress(m_CompressStruct);
            delete m_CompressStruct;
            delete m_ErrorHandler;
        }
    };
}
