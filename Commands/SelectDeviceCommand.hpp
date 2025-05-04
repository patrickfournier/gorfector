#pragma once

#include <utility>

#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SelectDeviceCommand
     * \brief Command class to select a device in the `DeviceSelectorState`.
     *
     * This class encapsulates the logic for selecting a device by its name
     * in the `DeviceSelectorState`.
     */
    class SelectDeviceCommand : public ZooLib::Command
    {
        /**
         * \brief The name of the device to be selected.
         */
        std::string m_DeviceName{};

    public:
        /**
         * \brief Constructor for the SelectDeviceCommand.
         * \param deviceName The name of the device to be selected.
         */
        explicit SelectDeviceCommand(std::string deviceName)
            : m_DeviceName(std::move(deviceName))
        {
        }

        /**
         * \brief Executes the command to select a device.
         * \param command The `SelectDeviceCommand` instance containing the device name.
         * \param deviceSelectorState Pointer to the `DeviceSelectorState` where the device will be selected.
         */
        static void Execute(const SelectDeviceCommand &command, DeviceSelectorState *deviceSelectorState)
        {
            const DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.SelectDevice(command.m_DeviceName);
        }
    };
}
