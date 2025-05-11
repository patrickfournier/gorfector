#pragma once

#include "DeviceSelectorState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class RefreshDeviceList
     * \brief Command class to refresh the list of devices in the `DeviceSelectorState`.
     *
     * This class encapsulates the logic for updating the device list managed by
     * the `DeviceSelectorState`.
     */
    class RefreshDeviceList : public ZooLib::Command
    {
    public:
        /**
         * \brief Executes the command to refresh the device list.
         * \param command The `RefreshDeviceList` instance (unused in this implementation).
         * \param deviceSelectorState Pointer to the `DeviceSelectorState` where the device list will be updated.
         */
        static void Execute(const RefreshDeviceList &command, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.UpdateDeviceList();
        }
    };
}
