#pragma once

#include "PreviewState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetZoomCommand
     * \brief Command class to set the zoom factor and center in the `PreviewState`.
     *
     * This class encapsulates the logic for updating the zoom factor and optionally
     * the zoom center in the `PreviewState`.
     */
    class SetZoomCommand : public ZooLib::Command
    {
        /**
         * \brief The desired zoom factor for the preview.
         */
        const double m_ZoomFactor{};

        /**
         * \brief The desired zoom center for the preview.
         */
        const Point<double> m_ZoomCenter{};

        /**
         * \brief Flag indicating whether to use the window center as the zoom center.
         */
        const bool m_UseWindowCenter{};

    public:
        /**
         * \brief Constructor for the `SetZoomCommand` with a specified zoom center.
         * \param zoomFactor The desired zoom factor to set.
         * \param zoomCenter The desired zoom center to set.
         */
        explicit SetZoomCommand(double zoomFactor, const Point<double> zoomCenter)
            : m_ZoomFactor(zoomFactor)
            , m_ZoomCenter(zoomCenter)
        {
        }

        /**
         * \brief Constructor for the `SetZoomCommand` using the window center.
         * \param zoomFactor The desired zoom factor to set.
         */
        explicit SetZoomCommand(double zoomFactor)
            : m_ZoomFactor(zoomFactor)
            , m_UseWindowCenter(true)
        {
        }

        /**
         * \brief Executes the command to set the zoom factor and center.
         * \param command The `SetZoomCommand` instance containing the desired zoom settings.
         * \param previewState Pointer to the `PreviewState` where the zoom settings will be updated.
         */
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
