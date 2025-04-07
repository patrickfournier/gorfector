#pragma once

#include "PreviewState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    struct SetPanCommand : public ZooLib::Command
    {
    private:
        const Point<double> m_PanOffset{};

    public:
        explicit SetPanCommand(const Point<double> panOffset)
            : m_PanOffset(panOffset)
        {
        }

        static void Execute(const SetPanCommand &command, PreviewState *previewState)
        {
            auto updater = PreviewState::Updater(previewState);
            updater.SetPanOffset(command.m_PanOffset);
        }
    };
}
