#pragma once

#include "DeviceOptionsState.hpp"

namespace ZooScan
{
    template<typename TValueType>
    struct ChangeOptionCommand : public ZooLib::Command
    {
    private:
        const uint32_t m_SettingIndex;
        const uint32_t m_ValueIndex;
        const TValueType m_Value;

    public:
        ChangeOptionCommand(uint32_t settingIndex, uint32_t valueIndex, TValueType value)
            : m_SettingIndex(settingIndex)
            , m_ValueIndex(valueIndex)
            , m_Value(value)
        {
        }

        static void Execute(const ChangeOptionCommand<bool> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
        }

        static void Execute(const ChangeOptionCommand<int> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
        }

        static void Execute(const ChangeOptionCommand<std::string> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
            ;
        }
    };

}
