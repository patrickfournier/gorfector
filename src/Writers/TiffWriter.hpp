#pragma once

#include <tiffio.h>

#include "DeviceOptionsState.hpp"
#include "FileWriter.hpp"
#include "TiffWriterState.hpp"

namespace Gorfector
{
    /**
     * @class TiffWriter
     * @brief A class for writing TIFF files, inheriting from FileWriter.
     *
     * This class provides functionality to create, write, and manage TIFF files.
     * It uses the libtiff library for handling TIFF file operations.
     */
    class TiffWriter final : public FileWriter
    {
        /**
         * @brief Supported file extensions for TIFF files.
         */
        static const std::vector<std::string> k_Extensions;

        /**
         * @brief The name of the writer, set to "TIFF".
         */
        static constexpr std::string k_Name = "TIFF";

        /**
         * @brief Pointer to the state component for managing TIFF-specific options.
         */
        TiffWriterState *m_StateComponent{};

        /**
         * @brief Pointer to the TIFF file being written.
         */
        TIFF *m_File{};

        /**
         * @brief Counter for the number of lines written to the TIFF file.
         */
        int m_LineCounter{};

    public:
        /**
         * @brief Constructs a TiffWriter object.
         * @param state Pointer to the ZooLib::State object.
         * @param applicationName Name of the application using the writer.
         */
        explicit TiffWriter(ZooLib::State *state, const std::string &applicationName)
            : FileWriter(applicationName)
        {
            m_StateComponent = new TiffWriterState(state);
        }

        /**
         * @brief Destructor for TiffWriter.
         *
         * Cleans up the state component.
         */
        ~TiffWriter() override
        {
            delete m_StateComponent;
        }

        /**
         * @brief Retrieves the state component.
         * @return Pointer to the TiffWriterState object.
         */
        [[nodiscard]] TiffWriterState *GetStateComponent() const
        {
            return m_StateComponent;
        }

        /**
         * @brief Retrieves the name of the writer.
         * @return A constant reference to the writer's name.
         */
        [[nodiscard]] const std::string &GetName() const override
        {
            return k_Name;
        }

        /**
         * @brief Retrieves the supported file extensions.
         * @return A vector of supported file extensions.
         */
        [[nodiscard]] std::vector<std::string> GetExtensions() const override
        {
            return k_Extensions;
        }

        /**
         * @brief Creates a new TIFF file and initializes it with the provided parameters.
         *
         * This method opens a TIFF file for writing, sets its metadata fields, and prepares it
         * for writing image data. It uses the libtiff library to handle file operations.
         *
         * @param path The file path where the TIFF file will be created.
         * @param deviceOptions Pointer to the `DeviceOptionsState` containing device-specific options.
         * @param parameters The `SANE_Parameters` structure containing image parameters.
         * @return An `Error` enum indicating the success or failure of the operation.
         */
        Error CreateFile(
                std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters) override
        {
            double xResolution = deviceOptions == nullptr               ? 0
                                 : deviceOptions->GetXResolution() == 0 ? deviceOptions->GetResolution()
                                                                        : deviceOptions->GetXResolution();
            double yResolution = deviceOptions == nullptr               ? 0
                                 : deviceOptions->GetYResolution() == 0 ? deviceOptions->GetResolution()
                                                                        : deviceOptions->GetYResolution();

            auto mode = "w";
            if (static_cast<uint64_t>(parameters.bytes_per_line) * static_cast<uint64_t>(parameters.lines) >=
                4 * 1024 * 1024 * 1024UL)
            {
                mode = "w8";
            }

            m_File = TIFFOpen(path.c_str(), mode);

            if (m_File == nullptr)
            {
                return Error::CannotOpenFile;
            }

            if (deviceOptions != nullptr)
            {
                TIFFSetField(m_File, TIFFTAG_MAKE, deviceOptions->GetDeviceVendor());
                TIFFSetField(m_File, TIFFTAG_MODEL, deviceOptions->GetDeviceModel());
            }

            TIFFSetField(m_File, TIFFTAG_SOFTWARE, GetApplicationName().c_str());

            TIFFSetField(m_File, TIFFTAG_IMAGEWIDTH, parameters.pixels_per_line);
            TIFFSetField(m_File, TIFFTAG_IMAGELENGTH, parameters.lines);
            TIFFSetField(m_File, TIFFTAG_SAMPLESPERPIXEL, parameters.format == SANE_FRAME_RGB ? 3 : 1);
            TIFFSetField(m_File, TIFFTAG_BITSPERSAMPLE, parameters.depth);
            TIFFSetField(m_File, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(m_File, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            if (parameters.format == SANE_FRAME_RGB)
            {
                TIFFSetField(m_File, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
            }
            else if (parameters.format == SANE_FRAME_GRAY)
            {
                if (parameters.depth == 1)
                {
                    TIFFSetField(m_File, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
                }
                else
                {
                    TIFFSetField(m_File, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
                }
            }

            TIFFSetField(m_File, TIFFTAG_COMPRESSION, m_StateComponent->GetTiffCompression());
            if (m_StateComponent->GetTiffCompression() == COMPRESSION_JPEG)
            {
                TIFFSetField(m_File, TIFFTAG_JPEGQUALITY, m_StateComponent->GetJpegQuality());
            }
            else if (m_StateComponent->GetTiffCompression() == COMPRESSION_ADOBE_DEFLATE)
            {
                TIFFSetField(m_File, TIFFTAG_ZIPQUALITY, m_StateComponent->GetDeflateCompressionLevel());
            }

            TIFFSetField(m_File, TIFFTAG_ROWSPERSTRIP, 128);
            TIFFSetField(m_File, TIFFTAG_XRESOLUTION, xResolution);
            TIFFSetField(m_File, TIFFTAG_YRESOLUTION, yResolution);
            TIFFSetField(m_File, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

            m_LineCounter = 0;

            return Error::None;
        }

        /**
         * @brief Appends image data to the TIFF file.
         *
         * This method writes a specified number of lines of image data to the TIFF file.
         * It uses the libtiff library's `TIFFWriteScanline` function to write each line.
         *
         * @param bytes Pointer to the image data to be written.
         * @param numberOfLines The number of lines to write to the TIFF file.
         * @param parameters The `SANE_Parameters` structure containing image parameters.
         * @return The total number of bytes written to the TIFF file.
         */
        uint32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, const SANE_Parameters &parameters) override
        {
            for (auto i = 0; i < numberOfLines; i++)
            {
                TIFFWriteScanline(m_File, bytes + i * parameters.bytes_per_line, m_LineCounter++, 0);
            }

            return numberOfLines * parameters.bytes_per_line;
        }

        /**
         * @brief Closes the currently open TIFF file.
         *
         * This method ensures that the TIFF file is properly closed and releases
         * the associated resources.
         */
        void CloseFile() override
        {
            TIFFClose(m_File);
            m_File = nullptr;
        }

        /**
         * @brief Cancels the current TIFF file operation.
         *
         * This method closes the TIFF file and resets the file pointer to `nullptr`.
         * It is used to abort the file operation.
         */
        void CancelFile() override
        {
            TIFFClose(m_File);
            m_File = nullptr;
        }
    };
}
