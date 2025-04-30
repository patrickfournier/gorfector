#pragma once

#include "Writers/PngWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{

    class SetPngCompressionLevel : public ZooLib::Command
    {
    private:
        const int m_CompressionLevel{};

    public:
        explicit SetPngCompressionLevel(int compressionLevel)
            : m_CompressionLevel(compressionLevel)
        {
        }

        static void Execute(const SetPngCompressionLevel &command, PngWriterState *pngWriterState)
        {
            auto updater = PngWriterState::Updater(pngWriterState);
            updater.SetCompressionLevel(command.m_CompressionLevel);
        }
    };

}
