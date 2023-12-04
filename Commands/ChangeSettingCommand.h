#pragma once

#include "DeviceSettingsState.h"

namespace ZooScan
{
    template<typename TValueType>
    class ChangeSettingCommand : public Zoo::Command
    {
    public:
        int SettingIndex;
        TValueType Value;

        ChangeSettingCommand(int settingIndex, TValueType value)
                : SettingIndex(settingIndex), Value(value)
        {
        }

        static void Execute(const ChangeSettingCommand<TValueType>& command, DeviceSettingsState *state)
        {
            auto updater = DeviceSettingsState::Updater(state);
            updater.SetValue(command.SettingIndex, command.Value);
            g_print("Setting changed: %d\n", command.SettingIndex);
        }
    };
}
