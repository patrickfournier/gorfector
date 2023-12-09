#pragma once

#include "DeviceSettingsState.h"

namespace ZooScan
{
    template<typename TValueType>
    class ChangeSettingCommand : public Zoo::Command
    {
    public:
        const SaneDevice *Device;
        uint32_t SettingIndex;
        uint32_t ValueIndex;
        TValueType Value;

        ChangeSettingCommand(const SaneDevice* device, uint32_t settingIndex, uint32_t valueIndex, TValueType value)
                : Device(device), SettingIndex(settingIndex), ValueIndex(valueIndex), Value(value)
        {
        }

        static void Execute(const ChangeSettingCommand<bool>& command, DeviceSettingsState *state)
        {
            int optionInfo;
            SANE_Bool value;
            auto *handle = command.Device->Handle();

            const SANE_Option_Descriptor *optionDescriptor = handle->GetOptionDescriptor(command.SettingIndex);
            if (optionDescriptor->type != SANE_TYPE_BOOL)
            {
                throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + optionDescriptor->title);
            }

            value = command.Value ? SANE_TRUE : SANE_FALSE;
            handle->SetOption(command.SettingIndex, &value, &optionInfo);

            bool reloadOptions = (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;

            auto updater = DeviceSettingsState::Updater(state);
            updater.SetValue<bool>(command.SettingIndex, command.ValueIndex, value, value, reloadOptions);
        }

        static void Execute(const ChangeSettingCommand<int>& command, DeviceSettingsState *state)
        {
            SANE_Word *value;
            SANE_Word *actualValue;
            int optionInfo;
            bool reloadOptions;
            auto *handle = command.Device->Handle();

            const SANE_Option_Descriptor *optionDescriptor = handle->GetOptionDescriptor(command.SettingIndex);
            if (optionDescriptor->type != SANE_TYPE_INT && optionDescriptor->type != SANE_TYPE_FIXED)
            {
                throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + optionDescriptor->title);
            }

            try
            {
                value = new SANE_Word[optionDescriptor->size / sizeof(SANE_Word)];

                if (optionDescriptor->size > SANE_Int(sizeof(SANE_Word)))
                {
                    handle->GetOption(command.SettingIndex, value);
                    value[command.ValueIndex] = command.Value;
                }
                else
                {
                    value[0] = command.Value;
                }

                handle->SetOption(command.SettingIndex, value, &optionInfo);

                reloadOptions = (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;
                if ((optionInfo & SANE_INFO_INEXACT) != 0)
                {
                    actualValue = new SANE_Word[optionDescriptor->size / sizeof(SANE_Word)];
                    handle->GetOption(command.SettingIndex, actualValue);
                }
                else
                {
                    actualValue = value;
                }
            }
            catch (...)
            {
                if (actualValue != value)
                    delete[] actualValue;
                delete[] value;
                throw;
            }

            auto updater = DeviceSettingsState::Updater(state);
            updater.SetValue<int>(command.SettingIndex, command.ValueIndex, value[command.ValueIndex], actualValue[command.ValueIndex], reloadOptions);
        }

        static void Execute(const ChangeSettingCommand<std::string>& command, DeviceSettingsState *state)
        {
            char *actualValue = nullptr;
            int optionInfo;
            bool reloadOptions;
            auto *handle = command.Device->Handle();

            const SANE_Option_Descriptor *optionDescriptor = handle->GetOptionDescriptor(command.SettingIndex);
            if (optionDescriptor->type != SANE_TYPE_STRING)
            {
                throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + optionDescriptor->title);
            }

            try
            {
                handle->SetOption(command.SettingIndex, (void *) command.Value.c_str(), &optionInfo);

                reloadOptions = (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;
                if (optionInfo & SANE_INFO_INEXACT)
                {
                    actualValue = new char[optionDescriptor->size];
                    handle->GetOption(command.SettingIndex, actualValue);
                }
            }
            catch (...)
            {
                delete[] actualValue;
                throw;
            }

            std::string actualValueStr = actualValue != nullptr ? actualValue : command.Value;
            auto updater = DeviceSettingsState::Updater(state);
            updater.SetValue<std::string>(command.SettingIndex, command.ValueIndex, command.Value, actualValueStr, reloadOptions);
        }
    };
}
