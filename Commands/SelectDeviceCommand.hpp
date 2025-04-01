#pragma once

#include "SaneDevice.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    struct SelectDeviceCommand : public ZooLib::Command
    {
    private:
        std::string m_DeviceName{};

    public:
        explicit SelectDeviceCommand(const std::string& deviceName)
                : m_DeviceName(deviceName)
        {}

        [[nodiscard]] const std::string &DeviceName() const
        {
            return m_DeviceName;
        }

        static void Execute(const SelectDeviceCommand &command, DeviceSelectorState *deviceSelectorState)
        {
            const DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.SelectDevice(command.DeviceName());
        }
    };
}
