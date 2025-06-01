#pragma once

#include <string>
#include <vector>

#include <jpeglib.h>

#include "FileWriter.hpp"
#include "JpegWriterState.hpp"

namespace Gorfector
{
    /**
     * \class JpegWriter
     * \brief A file writer implementation for handling JPEG image files.
     *
     * This class provides functionality to create, append data to, and manage JPEG files.
     * It uses the libjpeg library for compression and supports various image formats and depths.
     */
    class JpegWriter final : public FileWriter
    {
        /**
         * \brief Supported file extensions for JPEG files.
         */
        static const std::vector<std::string> k_Extensions;

        /**
         * \brief Name of the file writer.
         */
        static constexpr std::string k_Name = "JPEG";

        /**
         * \brief State component for managing JPEG writer-specific settings.
         */
        JpegWriterState *m_StateComponent{};

        /**
         * \brief JPEG compression structure used by libjpeg.
         */
        jpeg_compress_struct *m_CompressStruct{};

        /**
         * \brief Error handler for libjpeg operations.
         */
        jpeg_error_mgr *m_ErrorHandler{};

        /**
         * \brief File pointer for the output JPEG file.
         */
        FILE *m_File{};

    public:
        /**
         * \brief Constructor for the JpegWriter class.
         * \param state Pointer to the application state.
         * \param applicationName Name of the application using the file writer.
         */
        JpegWriter(ZooLib::State *state, const std::string &applicationName)
            : FileWriter(applicationName)
        {
            m_StateComponent = new JpegWriterState(state);
        }

        /**
         * \brief Destructor for the JpegWriter class.
         */
        ~JpegWriter() override
        {
            delete m_StateComponent;
        }

        /**
         * \brief Retrieves the state component for the JPEG writer.
         * \return A pointer to the JpegWriterState object.
         */
        [[nodiscard]] JpegWriterState *GetStateComponent() const
        {
            return m_StateComponent;
        }

        /**
         * \brief Retrieves the name of the file writer.
         * \return A constant reference to the name string.
         */
        [[nodiscard]] const std::string &GetName() const override
        {
            return k_Name;
        }

        /**
         * \brief Retrieves the supported file extensions for the JPEG writer.
         * \return A vector of strings containing the supported extensions.
         */
        [[nodiscard]] std::vector<std::string> GetExtensions() const override
        {
            return k_Extensions;
        }

        /**
         * \brief Creates a new JPEG file for writing.
         * \param path The file path to create.
         * \param deviceOptions Pointer to the device options state.
         * \param parameters SANE parameters for the file.
         * \return An error code indicating the result of the operation.
         */
        Error CreateFile(
                std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters) override
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
            jpeg_set_quality(m_CompressStruct, m_StateComponent->GetQuality(), FALSE);
            jpeg_start_compress(m_CompressStruct, TRUE);

            return Error::None;
        }

        /**
         * \brief Appends bytes to the JPEG file.
         * \param bytes Pointer to the byte data.
         * \param numberOfLines Number of lines to append.
         * \param parameters SANE parameters for the file.
         * \return The number of bytes appended.
         */
        size_t AppendBytes(SANE_Byte *bytes, uint32_t numberOfLines, const SANE_Parameters &parameters) override
        {
            if (numberOfLines == 0 || m_CompressStruct == nullptr)
                return 0;

            if (m_CompressStruct->image_height <= m_CompressStruct->next_scanline)
                return 0;

            auto numLinesToWrite =
                    std::min(numberOfLines, m_CompressStruct->image_height - m_CompressStruct->next_scanline);

            auto rowPointer = new JSAMPROW[numLinesToWrite];
            SANE_Byte *auxBuffer = nullptr;

            if (parameters.depth == 1)
            {
                // Convert 1-bit grayscale to 8-bit grayscale
                auxBuffer = new SANE_Byte[numLinesToWrite * parameters.pixels_per_line];
                for (auto i = 0U; i < numLinesToWrite; ++i)
                {
                    auto line = &bytes[i * parameters.bytes_per_line];
                    for (auto j = 0; j < parameters.pixels_per_line; ++j)
                    {
                        // 0 is white
                        auxBuffer[i * parameters.pixels_per_line + j] = line[j / 8] & (1 << (7 - (j % 8))) ? 0 : 255;
                    }
                    rowPointer[i] = &auxBuffer[i * parameters.pixels_per_line];
                }
            }
            else if (parameters.depth == 8)
            {
                for (auto i = 0U; i < numLinesToWrite; ++i)
                {
                    rowPointer[i] = &bytes[i * parameters.bytes_per_line];
                }
            }
            else if (parameters.depth == 16)
            {
                // ReSharper disable once CppDFAUnreachableCode
                constexpr auto offset = std::endian::native == std::endian::little ? 1 : 0;

                if (parameters.format == SANE_FRAME_GRAY)
                {
                    for (auto j = 0U; j < numLinesToWrite; ++j)
                    {
                        auto line = &bytes[j * parameters.bytes_per_line];
                        for (auto i = 0; i < parameters.pixels_per_line; ++i)
                        {
                            bytes[j * parameters.pixels_per_line + i] = line[2 * i + offset];
                        }

                        rowPointer[j] = &bytes[j * parameters.pixels_per_line];
                    }
                }
                else if (parameters.format == SANE_FRAME_RGB)
                {
                    for (auto j = 0U; j < numLinesToWrite; ++j)
                    {
                        auto line = &bytes[j * parameters.bytes_per_line];
                        for (auto i = 0; i < parameters.pixels_per_line; ++i)
                        {
                            bytes[3 * (j * parameters.pixels_per_line + i)] = line[6 * i + offset];
                            bytes[3 * (j * parameters.pixels_per_line + i) + 1] = line[6 * i + 2 + offset];
                            bytes[3 * (j * parameters.pixels_per_line + i) + 2] = line[6 * i + 4 + offset];
                        }

                        rowPointer[j] = &bytes[3 * j * parameters.pixels_per_line];
                    }
                }
            }

            jpeg_write_scanlines(m_CompressStruct, rowPointer, numLinesToWrite);

            delete[] rowPointer;
            delete[] auxBuffer;

            return numLinesToWrite * parameters.bytes_per_line;
        }

        /**
         * \brief Closes the JPEG file after writing.
         */
        void CloseFile() override
        {
            jpeg_finish_compress(m_CompressStruct);

            CancelFile();
        }

        /**
         * \brief Cancels the file writing operation and cleans up resources.
         */
        void CancelFile() override
        {
            if (m_File != nullptr)
            {
                fclose(m_File);
                m_File = nullptr;
            }

            if (m_CompressStruct != nullptr)
            {
                jpeg_destroy_compress(m_CompressStruct);
                delete m_CompressStruct;
                m_CompressStruct = nullptr;
            }

            delete m_ErrorHandler;
            m_ErrorHandler = nullptr;
        }
    };
}
