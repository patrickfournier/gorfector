#pragma once

#include "DeviceSelectorState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    struct RefreshDeviceList : public ZooLib::Command
    {
    public:
        static void Execute(const RefreshDeviceList &, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.UpdateDeviceList();
        }
    };

    struct ActivateNetworkScan : public ZooLib::Command
    {
    private:
        bool m_ScanNetwork{};

    public:
        explicit ActivateNetworkScan(bool activate)
        : m_ScanNetwork(activate)
        {
        }

        [[nodiscard]] bool ScanNetwork() const
        {
            return m_ScanNetwork;
        }

        static void Execute(const ActivateNetworkScan &command, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.SetScanNetwork(command.m_ScanNetwork);
        }
    };
}
