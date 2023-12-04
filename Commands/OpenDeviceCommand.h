#pragma once

#include "AppState.h"

namespace ZooScan
{
    struct OpenDeviceCommand : public Zoo::Command
    {
        const SANE_Device *m_Device{};

    public:
        explicit OpenDeviceCommand(const SANE_Device *device)
                : m_Device(device)
        {}

        [[nodiscard]] const SANE_Device *Device() const
        { return m_Device; }

        static void Execute(const OpenDeviceCommand &command, AppState *appState)
        {
            AppState::Updater updater(appState);
            updater.SetCurrentDevice(command.Device());
        }
    };
}
