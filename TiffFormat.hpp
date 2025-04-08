#pragma once
#include <tiffio.h>


#include "DeviceOptionsState.hpp"
#include "FileFormat.hpp"


namespace ZooScan
{
    class TiffFormat : public FileFormat
    {
        static const std::vector<std::string> k_Extensions;
        static constexpr std::string k_Name = "TIFF";

    public:
        ~TiffFormat() override = default;

        [[nodiscard]] const std::string &GetName() const override
        {
            return k_Name;
        }

        [[nodiscard]] std::vector<std::string> GetExtensions() const override
        {
            return k_Extensions;
        }

        void
        Save(std::filesystem::path &path, const DeviceOptionsState *deviceOptions, const SANE_Parameters &parameters,
             SANE_Byte *image) const override
        {
            int xResolution = deviceOptions == nullptr               ? 0
                              : deviceOptions->GetXResolution() == 0 ? deviceOptions->GetResolution()
                                                                     : deviceOptions->GetXResolution();
            int yResolution = deviceOptions == nullptr               ? 0
                              : deviceOptions->GetYResolution() == 0 ? deviceOptions->GetResolution()
                                                                     : deviceOptions->GetYResolution();

            TIFF *tifFile = TIFFOpen(path.c_str(), "w");

            TIFFSetField(tifFile, TIFFTAG_IMAGEWIDTH, parameters.pixels_per_line);
            TIFFSetField(tifFile, TIFFTAG_IMAGELENGTH, parameters.lines);
            TIFFSetField(tifFile, TIFFTAG_SAMPLESPERPIXEL, parameters.format == SANE_FRAME_RGB ? 3 : 1);
            TIFFSetField(tifFile, TIFFTAG_BITSPERSAMPLE, parameters.depth);
            TIFFSetField(tifFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(tifFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            TIFFSetField(
                    tifFile, TIFFTAG_PHOTOMETRIC,
                    parameters.format == SANE_FRAME_RGB ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
            TIFFSetField(tifFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
            TIFFSetField(tifFile, TIFFTAG_ROWSPERSTRIP, 1);
            TIFFSetField(tifFile, TIFFTAG_XRESOLUTION, xResolution);
            TIFFSetField(tifFile, TIFFTAG_YRESOLUTION, yResolution);
            TIFFSetField(tifFile, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

            for (auto i = 0; i < parameters.lines; i++)
            {
                TIFFWriteScanline(tifFile, image + i * parameters.bytes_per_line, i, 0);
            }

            TIFFClose(tifFile);
        }
    };
}
