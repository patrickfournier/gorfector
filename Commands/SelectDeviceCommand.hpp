#pragma once

#include "AppState.hpp"
#include "SaneDevice.hpp"
#include "ZooFW/Command.hpp"

namespace ZooScan
{
    struct SelectDeviceCommand : public Zoo::Command
    {
        SaneDevice *m_Device{};

    public:
        explicit SelectDeviceCommand(SaneDevice *device)
                : m_Device(device)
        {}

        [[nodiscard]] SaneDevice *Device() const
        { return m_Device; }

        static void Execute(const SelectDeviceCommand &command, AppState *appState)
        {
            AppState::Updater updater(appState);
            updater.SetCurrentDevice(command.Device());
        }
    };
}
