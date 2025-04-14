#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetTiffCompression : public ZooLib::Command
    {
    private:
        const int m_CompressionIndex{};

    public:
        explicit SetTiffCompression(int compressionIndex)
            : m_CompressionIndex(compressionIndex)
        {
        }

        static void Execute(const SetTiffCompression &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetCompression(static_cast<TiffWriterState::Compression>(command.m_CompressionIndex));
        }
    };

}
