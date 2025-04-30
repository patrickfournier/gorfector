#pragma once

#include "PreviewState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    struct SetZoomCommand : public ZooLib::Command
    {
    private:
        const double m_ZoomFactor{};
        const Point<double> m_ZoomCenter{};
        const bool m_UseWindowCenter{};

    public:
        explicit SetZoomCommand(double zoomFactor, const Point<double> zoomCenter)
            : m_ZoomFactor(zoomFactor)
            , m_ZoomCenter(zoomCenter)
        {
        }

        explicit SetZoomCommand(double zoomFactor)
            : m_ZoomFactor(zoomFactor)
            , m_UseWindowCenter(true)
        {
        }

        static void Execute(const SetZoomCommand &command, PreviewState *previewState)
        {
            auto updater = PreviewState::Updater(previewState);
            if (command.m_UseWindowCenter)
            {
                updater.SetZoomFactor(command.m_ZoomFactor);
            }
            else
            {
                updater.SetZoomFactor(command.m_ZoomFactor, command.m_ZoomCenter);
            }
        }
    };
}
