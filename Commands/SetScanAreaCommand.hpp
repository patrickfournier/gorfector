#pragma once

#include "DeviceOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetScanAreaCommand
     * \brief Command class to set the scan area in the `DeviceOptionsState`.
     *
     * This class encapsulates the logic for updating the scan area
     * in the `DeviceOptionsState`.
     */
    class SetScanAreaCommand : public ZooLib::Command
    {
        /**
         * \brief The desired scan area as a rectangle.
         */
        const Rect<double> m_ScanArea;

    public:
        /**
         * \brief Constructor for the SetScanAreaCommand.
         * \param scanArea The desired scan area to set, represented as a rectangle.
         */
        explicit SetScanAreaCommand(const Rect<double> &scanArea)
            : m_ScanArea(scanArea)
        {
        }

        /**
         * \brief Executes the command to set the scan area.
         * \param command The `SetScanAreaCommand` instance containing the desired scan area.
         * \param deviceOptionState Pointer to the `DeviceOptionsState` where the scan area will be updated.
         *
         * This method updates the top-left (TLX, TLY) and bottom-right (BRX, BRY) coordinates
         * of the scan area in the `DeviceOptionsState`.
         */
        static void Execute(const SetScanAreaCommand &command, DeviceOptionsState *deviceOptionState)
        {
            auto updater = DeviceOptionsState::Updater(deviceOptionState);
            updater.SetOptionValue(deviceOptionState->TLXIndex(), 0, command.m_ScanArea.x);
            updater.SetOptionValue(deviceOptionState->TLYIndex(), 0, command.m_ScanArea.y);
            updater.SetOptionValue(deviceOptionState->BRXIndex(), 0, command.m_ScanArea.x + command.m_ScanArea.width);
            updater.SetOptionValue(deviceOptionState->BRYIndex(), 0, command.m_ScanArea.y + command.m_ScanArea.height);
        }
    };
}
