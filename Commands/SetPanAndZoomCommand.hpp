#pragma once

#include "PreviewState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    struct SetPanAndZoomCommand : public ZooLib::Command
    {
    private:
        const Point<double> m_PanOffset{};
        const double m_ZoomFactor{};

    public:
        explicit SetPanAndZoomCommand(const Point<double> panOffset, double zoomFactor)
            : m_PanOffset(panOffset)
            , m_ZoomFactor(zoomFactor)
        {
        }

        static void Execute(const SetPanAndZoomCommand &command, PreviewState *previewState)
        {
            auto updater = PreviewState::Updater(previewState);
            updater.SetPanOffset(command.m_PanOffset);
            updater.SetZoomFactor(command.m_ZoomFactor);
        }
    };
}
