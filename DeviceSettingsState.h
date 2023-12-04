#pragma once

#include "ZooFW/StateComponent.h"
#include "DeviceSettingsValueBase.h"
#include "DeviceSettingValue.h"

namespace ZooScan
{
    class DeviceSettingsState : public Zoo::StateComponent
    {
        std::vector<DeviceSettingValueBase*> m_SettingValues;

    public:
        ~DeviceSettingsState()
        {
            for (auto settingValue : m_SettingValues)
            {
                delete settingValue;
            }
        }

        [[nodiscard]] SANE_Value_Type GetValueType(int settingIndex) const
        {
            return m_SettingValues[settingIndex]->ValueType();
        }

        template<typename TValueType>
        [[nodiscard]] TValueType GetValue(int settingIndex, int valueIndex = 0) const
        {
            return dynamic_cast<DeviceSettingValue<TValueType> const *>(m_SettingValues[settingIndex])->GetValue(valueIndex);
        }

        class Updater : public Zoo::StateComponent::Updater<DeviceSettingsState>
        {
        public:
            explicit Updater(DeviceSettingsState *state)
                    : Zoo::StateComponent::Updater<DeviceSettingsState>(state)
            {}

            void AddSettingValue(int index, DeviceSettingValueBase *settingValue)
            {
                if (m_StateComponent->m_SettingValues.size() <= index)
                {
                    m_StateComponent->m_SettingValues.resize(index + 1);
                }
                m_StateComponent->m_SettingValues[index] = settingValue;
            }

            template<typename TValueType>
            void SetValue(int settingIndex, TValueType value)
            {
                auto *setting = m_StateComponent->m_SettingValues[settingIndex];
                auto *typedSetting = dynamic_cast<DeviceSettingValue<TValueType> *>(setting);
                typedSetting->SetValue(0, value);
            }

            template<typename TValueType>
            void SetValue(int settingIndex, int valueIndex, TValueType value)
            {
                auto *setting = m_StateComponent->m_SettingValues[settingIndex];
                auto *typedSetting = dynamic_cast<DeviceSettingValue<TValueType> *>(setting);
                typedSetting->SetValue(valueIndex, value);
            }
        };
    };
}
