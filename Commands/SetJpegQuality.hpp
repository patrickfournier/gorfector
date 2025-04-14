#pragma once

#include "Writers/JpegWriterState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetJpegQuality : public ZooLib::Command
    {
    private:
        const int m_JpegQuality{};

    public:
        explicit SetJpegQuality(int jpegQuality)
            : m_JpegQuality(jpegQuality)
        {
        }

        static void Execute(const SetJpegQuality &command, JpegWriterState *jpegWriterState)
        {
            auto updater = JpegWriterState::Updater(jpegWriterState);
            updater.SetQuality(command.m_JpegQuality);
        }
    };

}
