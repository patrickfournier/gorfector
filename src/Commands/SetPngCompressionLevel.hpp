#pragma once

#include "Writers/PngWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetPngCompressionLevel
     * \brief Command class to set the PNG compression level in the `PngWriterState`.
     *
     * This class encapsulates the logic for updating the PNG compression level
     * in the `PngWriterState`.
     */
    class SetPngCompressionLevel : public ZooLib::Command
    {
        /**
         * \brief The desired PNG compression level.
         */
        const int m_CompressionLevel{};

    public:
        /**
         * \brief Constructor for the SetPngCompressionLevel command.
         * \param compressionLevel The desired compression level to set.
         */
        explicit SetPngCompressionLevel(int compressionLevel)
            : m_CompressionLevel(compressionLevel)
        {
        }

        /**
         * \brief Executes the command to set the PNG compression level.
         * \param command The `SetPngCompressionLevel` instance containing the desired compression level.
         * \param pngWriterState Pointer to the `PngWriterState` where the compression level will be updated.
         */
        static void Execute(const SetPngCompressionLevel &command, PngWriterState *pngWriterState)
        {
            auto updater = PngWriterState::Updater(pngWriterState);
            updater.SetCompressionLevel(command.m_CompressionLevel);
        }
    };
}
