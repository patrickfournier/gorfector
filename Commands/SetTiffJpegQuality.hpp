#pragma once

#include "Writers/TiffWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetTiffJpegQuality : public ZooLib::Command
    {
    private:
        const int m_JpegQuality{};

    public:
        explicit SetTiffJpegQuality(int jpegQuality)
            : m_JpegQuality(jpegQuality)
        {
        }

        static void Execute(const SetTiffJpegQuality &command, TiffWriterState *tiffWriterState)
        {
            auto updater = TiffWriterState::Updater(tiffWriterState);
            updater.SetJpegQuality(command.m_JpegQuality);
        }
    };

}
