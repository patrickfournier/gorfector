#pragma once
#include <tiffio.h>


#include "App.hpp"
#include "DeviceOptionsState.hpp"
#include "FileWriter.hpp"
#include "TiffWriterState.hpp"


namespace Gorfector
{
    class TiffWriter : public FileWriter
    {
        static const std::vector<std::string> k_Extensions;
        static constexpr std::string k_Name = "TIFF";

        TiffWriterState *m_StateComponent{};
        TIFF *m_File{};
        int m_LineCounter{};

    public:
        explicit TiffWriter(ZooLib::State *state, const std::string &applicationName)
            : FileWriter(applicationName)
        {
            m_StateComponent = new TiffWriterState(state);
        }

        ~TiffWriter() override
        {
            delete m_StateComponent;
        }

        [[nodiscard]] TiffWriterState *GetStateComponent()
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
                std::filesystem::path &path, const DeviceOptionsState *deviceOptions, const SANE_Parameters &parameters,
                SANE_Byte *image) override
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
            else if (m_StateComponent->GetTiffCompression() == COMPRESSION_DEFLATE)
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

        int32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, const SANE_Parameters &parameters) override
        {
            for (auto i = 0; i < numberOfLines; i++)
            {
                TIFFWriteScanline(m_File, bytes + i * parameters.bytes_per_line, m_LineCounter++, 0);
            }

            return numberOfLines * parameters.bytes_per_line;
        }

        void CloseFile() override
        {
            TIFFClose(m_File);
            m_File = nullptr;
        }

        void CancelFile() override
        {
            TIFFClose(m_File);
            m_File = nullptr;
        }
    };
}
