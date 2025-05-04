#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetTiffDeflateLevel
     * \brief Command class to set the deflate compression level in the `TiffWriterState`.
     *
     * This class encapsulates the logic for updating the deflate compression level
     * in the `TiffWriterState`.
     */
    class SetTiffDeflateLevel : public ZooLib::Command
    {
        /**
         * \brief The desired deflate compression level for the TIFF writer.
         */
        const int m_DeflateCompressionLevel{};

    public:
        /**
         * \brief Constructor for the SetTiffDeflateLevel command.
         * \param deflateCompressionLevel The desired deflate compression level to set.
         */
        explicit SetTiffDeflateLevel(int deflateCompressionLevel)
            : m_DeflateCompressionLevel(deflateCompressionLevel)
        {
        }

        /**
         * \brief Executes the command to set the deflate compression level.
         * \param command The `SetTiffDeflateLevel` instance containing the desired compression level.
         * \param tiffWriterState Pointer to the `TiffWriterState` where the compression level will be updated.
         */
        static void Execute(const SetTiffDeflateLevel &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetDeflateCompressionLevel(command.m_DeflateCompressionLevel);
        }
    };
}
