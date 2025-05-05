#pragma once

#include <png.h>
#include <zlib.h>

#include "App.hpp"
#include "FileWriter.hpp"
#include "PngWriterState.hpp"

namespace Gorfector
{
    /**
     * \class PngWriter
     * \brief A class for writing PNG files using the libpng library.
     *
     * This class provides functionality to create, write, and manage PNG files.
     */
    class PngWriter final : public FileWriter
    {
        /**
         * \brief Supported file extensions for PNG files.
         */
        static const std::vector<std::string> k_Extensions;

        /**
         * \brief The name of the writer.
         */
        static constexpr std::string k_Name = "PNG";

        /**
         * \brief Pointer to the state component managing PNG writer state.
         */
        PngWriterState *m_StateComponent{};

        /**
         * \brief File pointer for the output PNG file.
         */
        FILE *m_File{};

        /**
         * \brief Pointer to the libpng write structure.
         */
        png_structp m_Png{};

        /**
         * \brief Pointer to the libpng info structure.
         */
        png_infop m_PngInfo{};

        /**
         * \brief Array of line pointers for writing image data.
         */
        SANE_Byte **m_LinePointers{};

        /**
         * \brief Size of the line pointer array.
         */
        int m_LinePointerSize{};

    public:
        /**
         * \brief Constructor for the PngWriter class.
         * \param state Pointer to the ZooLib state object.
         * \param applicationName Name of the application using the writer.
         */
        PngWriter(ZooLib::State *state, const std::string &applicationName)
            : FileWriter(applicationName)
        {
            m_StateComponent = new PngWriterState(state);
        }

        /**
         * \brief Destructor for the PngWriter class.
         */
        ~PngWriter() override
        {
            delete m_StateComponent;
        }

        /**
         * \brief Retrieves the state component associated with the writer.
         * \return Pointer to the PngWriterState object.
         */
        [[nodiscard]] PngWriterState *GetStateComponent() const
        {
            return m_StateComponent;
        }

        /**
         * \brief Retrieves the name of the writer.
         * \return A constant reference to the writer's name.
         */
        [[nodiscard]] const std::string &GetName() const override
        {
            return k_Name;
        }

        /**
         * \brief Retrieves the supported file extensions for the writer.
         * \return A vector of supported file extensions.
         */
        [[nodiscard]] std::vector<std::string> GetExtensions() const override
        {
            return k_Extensions;
        }

        /**
         * \brief Creates a new PNG file.
         * \param path Path to the file to be created.
         * \param deviceOptions Pointer to device options state.
         * \param parameters Parameters for the image to be written.
         * \return An error code indicating the result of the operation.
         */
        Error CreateFile(
                std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters) override
        {
            if (parameters.depth == 1)
            {
                png_set_invert_mono(m_Png);
            }

            std::string appId = GetApplicationName();
            std::string creator = std::string(deviceOptions->GetDeviceVendor()) + " " + deviceOptions->GetDeviceModel();

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

        /**
         * \brief Appends image data to the PNG file.
         * \param bytes Pointer to the image data.
         * \param numberOfLines Number of lines to append.
         * \param parameters Parameters for the image data.
         * \return The number of bytes successfully written.
         */
        uint32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, const SANE_Parameters &parameters) override
        {
            if (m_LinePointers == nullptr || m_LinePointerSize < numberOfLines)
            {
                delete[] m_LinePointers;
                m_LinePointers = new SANE_Byte *[numberOfLines];
                m_LinePointerSize = numberOfLines;
            }

            int i = 0;

            if (setjmp(png_jmpbuf(m_Png))) // NOLINT(*-err52-cpp)
            {
                return i * parameters.bytes_per_line;
            }

            for (i = 0; i < numberOfLines; ++i)
            {
                m_LinePointers[i] = bytes + i * parameters.bytes_per_line;
            }
            png_write_rows(m_Png, m_LinePointers, m_LinePointerSize);

            return numberOfLines * parameters.bytes_per_line;
        }

        /**
         * \brief Closes the PNG file, finalizing the write process.
         */
        void CloseFile() override
        {
            if (setjmp(png_jmpbuf(m_Png))) // NOLINT(*-err52-cpp)
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

        /**
         * \brief Cancels the file writing process and cleans up resources.
         */
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
