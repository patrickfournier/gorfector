#pragma once

#include "ZooFW/StateComponent.h"
#include "DeviceSettingsValueBase.h"
#include "DeviceSettingValue.h"
#include "ZooFW/ChangesetManager.h"
#include "ZooFW/ChangesetBase.h"

namespace ZooScan
{
    inline uint64_t WidgetIndex(uint32_t settingIndex, uint32_t valueIndex)
    {
        return (uint64_t(settingIndex) << 32) | valueIndex;
    }

    inline uint32_t SettingIndex(uint64_t widgetIndex)
    {
        return uint32_t(widgetIndex >> 32);
    }

    inline uint32_t ValueIndex(uint64_t widgetIndex)
    {
        return uint32_t(widgetIndex);
    }

    class Changeset : public Zoo::ChangesetBase
    {
        std::vector<uint64_t> m_ChangedIndices;
        bool m_ReloadOptions{};

    public:
        explicit Changeset(uint64_t stateInitialVersion)
        : ChangesetBase(stateInitialVersion)
        {
        }

        void AddChangedIndex(uint64_t index)
        {
            m_ChangedIndices.push_back(index);
        }

        [[nodiscard]] const std::vector<uint64_t> &ChangedIndices() const
        {
            return m_ChangedIndices;
        }

        void SetReloadOptions(bool reloadOptions)
        {
            m_ReloadOptions = reloadOptions;
        }

        [[nodiscard]] bool ReloadOptions() const
        {
            return m_ReloadOptions;
        }

        void Aggregate(const Changeset& changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ReloadOptions |= changeset.m_ReloadOptions;

            for (auto changedIndex : changeset.m_ChangedIndices)
            {
                if (std::find(m_ChangedIndices.begin(), m_ChangedIndices.end(), changedIndex) == m_ChangedIndices.end())
                {
                    m_ChangedIndices.push_back(changedIndex);
                }
            }
        }
    };

    class DeviceSettingsState : public Zoo::StateComponent, public Zoo::ChangesetManager<Changeset>
    {
        std::vector<DeviceSettingValueBase*> m_SettingValues;

        [[nodiscard]] Changeset * GetCurrentChangeset()
        {
            return ChangesetManager::GetCurrentChangeset(Version());
        }

    public:
        ~DeviceSettingsState() override
        {
            for (auto settingValue : m_SettingValues)
            {
                delete settingValue;
            }
        }

        [[nodiscard]] SANE_Value_Type GetValueType(uint32_t settingIndex) const
        {
            return m_SettingValues[settingIndex]->ValueType();
        }

        template<typename TValueType>
        [[nodiscard]] TValueType GetValue(uint32_t settingIndex, uint32_t valueIndex = 0) const
        {
            return dynamic_cast<DeviceSettingValue<TValueType> const *>(m_SettingValues[settingIndex])->GetValue(valueIndex);
        }

        class Updater : public Zoo::StateComponent::Updater<DeviceSettingsState>
        {
        public:
            explicit Updater(DeviceSettingsState *state)
                    : Zoo::StateComponent::Updater<DeviceSettingsState>(state)
            {}

            ~Updater() override
            {
                m_StateComponent->PushCurrentChangeset();
            }

            void ClearSettings()
            {
                for (auto settingValue : m_StateComponent->m_SettingValues)
                {
                    delete settingValue;
                }
                m_StateComponent->m_SettingValues.clear();
            }

            void AddSettingValue(uint32_t index, DeviceSettingValueBase *settingValue)
            {
                if (m_StateComponent->m_SettingValues.size() <= index)
                {
                    m_StateComponent->m_SettingValues.resize(index + 1);
                }
                m_StateComponent->m_SettingValues[index] = settingValue;
            }

            template<typename TValueType>
            void InitValue(uint32_t settingIndex, const TValueType& value)
            {
                InitValue(settingIndex, 0, value);
            }

            template<typename TValueType>
            void InitValue(uint32_t settingIndex, uint32_t valueIndex, const TValueType& value)
            {
                auto *setting = m_StateComponent->m_SettingValues[settingIndex];
                auto *typedSetting = dynamic_cast<DeviceSettingValue<TValueType> *>(setting);
                typedSetting->SetValue(valueIndex, value);
            }

            template<typename TValueType>
            void SetValue(uint32_t settingIndex, TValueType requestedValue, const TValueType& value, bool reloadOptions = false)
            {
                SetValue(settingIndex, 0, requestedValue, value, reloadOptions);
            }

            template<typename TValueType>
            void SetValue(uint32_t settingIndex, uint32_t valueIndex, const TValueType& requestedValue, const TValueType& value, bool reloadOptions = false)
            {
                auto *setting = m_StateComponent->m_SettingValues[settingIndex];
                auto *typedSetting = dynamic_cast<DeviceSettingValue<TValueType> *>(setting);
                typedSetting->SetValue(valueIndex, requestedValue, value);

                if (reloadOptions)
                {
                    m_StateComponent->GetCurrentChangeset()->SetReloadOptions(true);
                }
                else
                {
                    m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(settingIndex, valueIndex));
                }
            }
        };
    };
}
