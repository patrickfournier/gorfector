#pragma once

#include <sane/sane.h>

class ImageGenerator
{
public:
    static void Generate1BitImage(
            unsigned int width, unsigned int height, SANE_Parameters *params, SANE_Byte **buffer, size_t *bufferSize)
    {
        params->format = SANE_FRAME_GRAY;
        params->last_frame = SANE_FALSE;
        params->depth = 1;
        params->bytes_per_line = (static_cast<int>(width) + 7) / 8; // 1 bit per pixel, rounded up to the nearest byte
        params->pixels_per_line = static_cast<int>(width);
        params->lines = static_cast<int>(height);

        *bufferSize = static_cast<size_t>(params->bytes_per_line) * height;
        *buffer = new SANE_Byte[*bufferSize];
        // Clear the buffer
        std::fill_n(*buffer, *bufferSize, 0);

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                size_t index = (y * params->bytes_per_line) + (x / 8);
                (*buffer)[index] |= ((x + y) % 2) << (7 - (x % 8)); // Alternate between 0 and 1
            }
        }
    }

    static void Generate8BitColorImage(
            unsigned int width, unsigned int height, SANE_Parameters *params, SANE_Byte **buffer, size_t *bufferSize)
    {
        params->format = SANE_FRAME_RGB;
        params->last_frame = SANE_FALSE;
        params->depth = 8;
        params->bytes_per_line = static_cast<int>(width) * 3; // 3 bytes per pixel for RGB
        params->pixels_per_line = static_cast<int>(width);
        params->lines = static_cast<int>(height);

        *bufferSize = static_cast<size_t>(params->bytes_per_line) * height;
        *buffer = new SANE_Byte[*bufferSize];

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                size_t index = (y * params->bytes_per_line) + (x * 3);
                (*buffer)[index] = static_cast<SANE_Byte>((x + y) % 256); // Red
                (*buffer)[index + 1] = static_cast<SANE_Byte>((x * 2 + y) % 256); // Green
                (*buffer)[index + 2] = static_cast<SANE_Byte>((x + y * 2) % 256); // Blue
            }
        }
    }

    static void Generate8BitGrayscaleImage(
            unsigned int width, unsigned int height, SANE_Parameters *params, SANE_Byte **buffer, size_t *bufferSize)
    {
        params->format = SANE_FRAME_GRAY;
        params->last_frame = SANE_FALSE;
        params->depth = 8;
        params->bytes_per_line = static_cast<int>(width); // 1 byte per pixel for grayscale
        params->pixels_per_line = static_cast<int>(width);
        params->lines = static_cast<int>(height);

        *bufferSize = static_cast<size_t>(params->bytes_per_line) * height;
        *buffer = new SANE_Byte[*bufferSize];

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                (*buffer)[y * params->bytes_per_line + x] = static_cast<SANE_Byte>((x + y) % 256); // Grayscale value
            }
        }
    }

    static void Generate16BitGrayscaleImage(
            unsigned int width, unsigned int height, SANE_Parameters *params, SANE_Byte **buffer, size_t *bufferSize)
    {
        params->format = SANE_FRAME_GRAY;
        params->last_frame = SANE_FALSE;
        params->depth = 16;
        params->bytes_per_line = static_cast<int>(width) * 2; // 2 bytes per pixel for 16-bit grayscale
        params->pixels_per_line = static_cast<int>(width);
        params->lines = static_cast<int>(height);

        *bufferSize = static_cast<size_t>(params->bytes_per_line) * height;
        *buffer = new SANE_Byte[*bufferSize];

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                size_t index = (y * params->bytes_per_line) + (x * 2);
                (*buffer)[index] = static_cast<SANE_Byte>(((x + y) / 256) % 256); // Low byte: low importance
                (*buffer)[index + 1] = static_cast<SANE_Byte>((x + y) % 256); // High byte: high importance
            }
        }
    }

    static void Generate16BitColorImage(
            unsigned int width, unsigned int height, SANE_Parameters *params, SANE_Byte **buffer, size_t *bufferSize)
    {
        params->format = SANE_FRAME_RGB;
        params->last_frame = SANE_FALSE;
        params->depth = 16;
        params->bytes_per_line = static_cast<int>(width) * 6; // 6 bytes per pixel for RGB (2 bytes each for R, G, B)
        params->pixels_per_line = static_cast<int>(width);
        params->lines = static_cast<int>(height);

        *bufferSize = static_cast<size_t>(params->bytes_per_line) * height;
        *buffer = new SANE_Byte[*bufferSize];

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                size_t index = (y * params->bytes_per_line) + (x * 6);
                (*buffer)[index] = static_cast<SANE_Byte>(((x + y) / 256) % 256); // Red low byte: low importance
                (*buffer)[index + 1] = static_cast<SANE_Byte>((x + y) % 256); // Red high byte: high importance
                (*buffer)[index + 2] =
                        static_cast<SANE_Byte>(((x * 2 + y) / 256) % 256); // Green low byte: low importance
                (*buffer)[index + 3] = static_cast<SANE_Byte>((x * 2 + y) % 256); // Green high byte: high importance
                (*buffer)[index + 4] =
                        static_cast<SANE_Byte>(((x + y * 2) / 256) % 256); // Blue low byte: low importance
                (*buffer)[index + 5] = static_cast<SANE_Byte>((x + y * 2) % 256); // Blue high byte: high importance
            }
        }
    }
};
