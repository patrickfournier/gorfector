#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{

    class SetTiffDeflateLevel : public ZooLib::Command
    {
    private:
        const int m_DeflateCompressionLevel{};

    public:
        explicit SetTiffDeflateLevel(int deflateCompressionLevel)
            : m_DeflateCompressionLevel(deflateCompressionLevel)
        {
        }

        static void Execute(const SetTiffDeflateLevel &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetDeflateCompressionLevel(command.m_DeflateCompressionLevel);
        }
    };

}
