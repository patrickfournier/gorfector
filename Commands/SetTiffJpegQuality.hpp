#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetTiffJpegQuality
     * \brief Command class to set the JPEG quality in the `TiffWriterState`.
     *
     * This class encapsulates the logic for updating the JPEG quality setting
     * in the `TiffWriterState`.
     */
    class SetTiffJpegQuality : public ZooLib::Command
    {
        /**
         * \brief The desired JPEG quality for the TIFF writer.
         */
        const int m_JpegQuality{};

    public:
        /**
         * \brief Constructor for the SetTiffJpegQuality command.
         * \param jpegQuality The desired JPEG quality to set.
         */
        explicit SetTiffJpegQuality(int jpegQuality)
            : m_JpegQuality(jpegQuality)
        {
        }

        /**
         * \brief Executes the command to set the JPEG quality.
         * \param command The `SetTiffJpegQuality` instance containing the desired JPEG quality.
         * \param tiffWriterState Pointer to the `TiffWriterState` where the JPEG quality will be updated.
         */
        static void Execute(const SetTiffJpegQuality &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetJpegQuality(command.m_JpegQuality);
        }
    };
}
