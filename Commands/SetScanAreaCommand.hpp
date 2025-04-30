#pragma once

#include "DeviceOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    struct SetScanAreaCommand : public ZooLib::Command
    {
    private:
        const Rect<double> m_ScanArea;

    public:
        explicit SetScanAreaCommand(const Rect<double> &scanArea)
            : m_ScanArea(scanArea)
        {
        }

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
