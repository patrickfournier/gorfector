#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetTiffCompression
     * \brief Command class to set the TIFF compression in the `TiffWriterState`.
     *
     * This class encapsulates the logic for updating the compression setting
     * in the `TiffWriterState`.
     */
    class SetTiffCompression : public ZooLib::Command
    {
        /**
         * \brief The desired compression index for the TIFF writer.
         */
        const int m_CompressionIndex{};

    public:
        /**
         * \brief Constructor for the SetTiffCompression command.
         * \param compressionIndex The desired compression index to set.
         */
        explicit SetTiffCompression(int compressionIndex)
            : m_CompressionIndex(compressionIndex)
        {
        }

        /**
         * \brief Executes the command to set the TIFF compression.
         * \param command The `SetTiffCompression` instance containing the desired compression index.
         * \param tiffWriterState Pointer to the `TiffWriterState` where the compression will be updated.
         */
        static void Execute(const SetTiffCompression &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetCompression(static_cast<TiffWriterState::Compression>(command.m_CompressionIndex));
        }
    };
}
