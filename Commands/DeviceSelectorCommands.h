#pragma once

#include "DeviceSelectorState.h"

namespace ZooScan
{
    struct RefreshDeviceList : public Zoo::Command
    {
    public:
        static void Execute(const RefreshDeviceList &, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.UpdateDeviceList();
        }
    };

    struct ActivateNetworkScan : public Zoo::Command
    {
    public:
        bool ScanNetwork;

        static void Execute(const ActivateNetworkScan &command, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.SetScanNetwork(command.ScanNetwork);
        }
    };
}
