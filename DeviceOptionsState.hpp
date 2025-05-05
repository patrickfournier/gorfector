#pragma once

#include <cstring>
#include <nlohmann/json.hpp>
#include <utility>

#include "DeviceOptionValue.hpp"
#include "DeviceOptionValueBase.hpp"
#include "DeviceSelectorState.hpp"
#include "Rect.hpp"
#include "ZooLib/ChangesetBase.hpp"
#include "ZooLib/ChangesetManager.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class SaneDevice;

    enum class ScanAreaUnit
    {
        Pixels,
        Millimeters
    };

    union WidgetIndex
    {
        const uint32_t OptionValueIndices[2];
        const uint64_t CompositeIndex;
    };

    /**
     * \brief A changeset class for managing changes in device options state.
     */
    class DeviceOptionsStateChangeset : public ZooLib::ChangesetBase
    {
        std::vector<WidgetIndex> m_ChangedIndices;
        bool m_ReloadOptions{};

    public:
        explicit DeviceOptionsStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        void AddChangedIndex(WidgetIndex index)
        {
            m_ChangedIndices.push_back(index);
        }

        [[nodiscard]] const std::vector<WidgetIndex> &GetChangedIndices() const
        {
            return m_ChangedIndices;
        }

        void SetReloadOptions(bool reloadOptions)
        {
            m_ReloadOptions = reloadOptions;
        }

        [[nodiscard]] bool RebuildAll() const
        {
            return m_ReloadOptions;
        }

        void Aggregate(const DeviceOptionsStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ReloadOptions |= changeset.m_ReloadOptions;

            for (auto changedIndex: changeset.m_ChangedIndices)
            {
                if (std::ranges::find_if(m_ChangedIndices, [changedIndex](const WidgetIndex &a) {
                        return a.CompositeIndex == changedIndex.CompositeIndex;
                    }) == m_ChangedIndices.end())
                {
                    m_ChangedIndices.push_back(changedIndex);
                }
            }
        }
    };

    class DeviceOptionsState : public ZooLib::StateComponent,
                               public ZooLib::ChangesetManager<DeviceOptionsStateChangeset>
    {
    public:
        static constexpr uint32_t k_InvalidIndex = std::numeric_limits<uint32_t>::max();
        static constexpr const char *k_DeviceKey = "Device";
        static constexpr const char *k_DeviceNameKey = "Name";
        static constexpr const char *k_DeviceVendorKey = "Vendor";
        static constexpr const char *k_DeviceModelKey = "Model";
        static constexpr const char *k_DeviceTypeKey = "Type";
        static constexpr const char *k_OptionsKey = "Options";
        static constexpr const char *k_TlxKey = "tl-x";
        static constexpr const char *k_TlyKey = "tl-y";
        static constexpr const char *k_BrxKey = "br-x";
        static constexpr const char *k_BryKey = "br-y";

    private:
        const std::string m_DeviceName;
        std::vector<DeviceOptionValueBase *> m_OptionValues;

        uint32_t m_PreviewIndex;
        uint32_t m_ModeIndex;
        uint32_t m_TLXIndex;
        uint32_t m_TLYIndex;
        uint32_t m_BRXIndex;
        uint32_t m_BRYIndex;
        uint32_t m_ResolutionIndex;
        uint32_t m_XResolutionIndex;
        uint32_t m_YResolutionIndex;
        uint32_t m_BitDepthIndex;

        [[nodiscard]] DeviceOptionsStateChangeset *GetCurrentChangeset()
        {
            return ChangesetManager::GetCurrentChangeset(GetVersion());
        }

        [[nodiscard]] SaneDevice *GetDevice() const
        {
            if (m_DeviceName.empty() || m_State == nullptr)
            {
                return nullptr;
            }

            const auto deviceSelectorState = m_State->GetStateComponentByType<DeviceSelectorState>();
            if (deviceSelectorState == nullptr)
            {
                return nullptr;
            }
            return deviceSelectorState->GetDeviceByName(m_DeviceName);
        }

        void ReloadOptions() const;
        void BuildOptions();

        void AddOptionValue(uint32_t index, DeviceOptionValueBase *optionValue)
        {
            if (m_OptionValues.size() <= index)
            {
                m_OptionValues.resize(index + 1);
            }
            m_OptionValues[index] = optionValue;

            /* Save special indices */

            if (strcmp(optionValue->GetName(), "preview") == 0)
            {
                m_PreviewIndex = index;
            }
            else if (strcmp(optionValue->GetName(), "mode") == 0) // not a "well-known" option
            {
                m_ModeIndex = index;
            }
            else
            {
                if (strcmp(optionValue->GetName(), k_TlxKey) == 0)
                {
                    m_TLXIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_TlyKey) == 0)
                {
                    m_TLYIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_BrxKey) == 0)
                {
                    m_BRXIndex = index;
                }
                else if (strcmp(optionValue->GetName(), k_BryKey) == 0)
                {
                    m_BRYIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "resolution") == 0)
                {
                    m_ResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "x-resolution") == 0) // not a "well-known" option
                {
                    m_XResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "y-resolution") == 0) // not a "well-known" option
                {
                    m_YResolutionIndex = index;
                }
                else if (strcmp(optionValue->GetName(), "depth") == 0) // not a "well-known" option
                {
                    m_BitDepthIndex = index;
                }
            }
        }

        void Clear()
        {
            for (auto optionValue: m_OptionValues)
            {
                delete optionValue;
            }
            m_OptionValues.clear();
        }

        friend void to_json(nlohmann::json &j, const DeviceOptionsState &p);

    public:
        DeviceOptionsState(ZooLib::State *state, std::string deviceName)
            : StateComponent(state)
            , m_DeviceName(std::move(deviceName))
            , m_PreviewIndex(k_InvalidIndex)
            , m_ModeIndex(k_InvalidIndex)
            , m_TLXIndex(k_InvalidIndex)
            , m_TLYIndex(k_InvalidIndex)
            , m_BRXIndex(k_InvalidIndex)
            , m_BRYIndex(k_InvalidIndex)
            , m_ResolutionIndex(k_InvalidIndex)
            , m_XResolutionIndex(k_InvalidIndex)
            , m_YResolutionIndex(k_InvalidIndex)
            , m_BitDepthIndex(k_InvalidIndex)
        {
            BuildOptions();
        }

        ~DeviceOptionsState() override
        {
            Clear();
        }

        class Updater : public StateComponent::Updater<DeviceOptionsState>
        {
            void ApplyRequestedValuesToDevice(const std::vector<size_t> &changedIndices);
            void FindRequestMismatches(const std::vector<size_t> &indicesToCheck, std::vector<size_t> &mismatches);
            void ApplyPreset(const nlohmann::json &json);

        public:
            explicit Updater(DeviceOptionsState *state)
                : StateComponent::Updater<DeviceOptionsState>(state)
            {
            }

            ~Updater() override
            {
                m_StateComponent->PushCurrentChangeset();
            }

            void LoadFromJson(const nlohmann::json &json) override;
            void ApplySettings(const nlohmann::json &json)
            {
                LoadFromJson(json);
            }

            void ApplyScanArea(const nlohmann::json &json)
            {
                ApplyPreset(json);
            }

            void ReloadOptions() const
            {
                m_StateComponent->ReloadOptions();
                m_StateComponent->GetCurrentChangeset()->SetReloadOptions(true);
            }

            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, bool requestedValue) const;
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, double requestedValue) const;
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, int requestedValue) const;
            void SetOptionValue(uint32_t optionIndex, uint32_t valueIndex, const std::string &requestedValue) const;
        };

        [[nodiscard]] const char *GetDeviceModel() const
        {
            auto saneDevice = GetDevice();
            if (saneDevice == nullptr)
            {
                return nullptr;
            }

            return saneDevice->GetModel();
        }

        [[nodiscard]] const char *GetDeviceVendor() const
        {
            auto saneDevice = GetDevice();
            if (saneDevice == nullptr)
            {
                return nullptr;
            }

            return saneDevice->GetVendor();
        }

        [[nodiscard]] size_t GetOptionCount() const
        {
            return m_OptionValues.size();
        }

        [[nodiscard]] const DeviceOptionValueBase *GetOption(uint32_t settingIndex) const
        {
            return m_OptionValues[settingIndex];
        }

        template<typename T>
        [[nodiscard]] const DeviceOptionValue<T> *GetOption(uint32_t settingIndex) const
        {
            return dynamic_cast<const DeviceOptionValue<T> *>(m_OptionValues[settingIndex]);
        }

        [[nodiscard]] bool IsPreview() const;

        [[nodiscard]] std::string GetMode() const;

        [[nodiscard]] ScanAreaUnit GetScanAreaUnit() const;

        [[nodiscard]] Rect<double> GetScanArea() const;

        [[nodiscard]] Rect<double> GetMaxScanArea() const;

        [[nodiscard]] int GetResolution() const;

        [[nodiscard]] int GetXResolution() const;

        [[nodiscard]] int GetYResolution() const;

        [[nodiscard]] int GetBitDepth() const;

        [[nodiscard]] uint32_t PreviewIndex() const
        {
            return m_PreviewIndex;
        }

        [[nodiscard]] uint32_t ModeIndex() const
        {
            return m_ModeIndex;
        }

        [[nodiscard]] uint32_t TLXIndex() const
        {
            return m_TLXIndex;
        }

        [[nodiscard]] uint32_t TLYIndex() const
        {
            return m_TLYIndex;
        }

        [[nodiscard]] uint32_t BRXIndex() const
        {
            return m_BRXIndex;
        }

        [[nodiscard]] uint32_t BRYIndex() const
        {
            return m_BRYIndex;
        }

        [[nodiscard]] uint32_t ResolutionIndex() const
        {
            return m_ResolutionIndex;
        }

        [[nodiscard]] uint32_t XResolutionIndex() const
        {
            return m_XResolutionIndex;
        }

        [[nodiscard]] uint32_t YResolutionIndex() const
        {
            return m_YResolutionIndex;
        }

        [[nodiscard]] uint32_t BitDepthIndex() const
        {
            return m_BitDepthIndex;
        }
    };

    inline void to_json(nlohmann::json &j, const DeviceOptionsState &p)
    {
        auto device = p.GetDevice();
        if (device == nullptr)
        {
            return;
        }

        j = nlohmann::json::object();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceNameKey] = device->GetName();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceVendorKey] = device->GetVendor();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceModelKey] = device->GetModel();
        j[DeviceOptionsState::k_DeviceKey][DeviceOptionsState::k_DeviceTypeKey] = device->GetType();

        j[DeviceOptionsState::k_OptionsKey] = nlohmann::json::object();
        for (auto optionValue: p.m_OptionValues)
        {
            if (optionValue == nullptr)
            {
                continue;
            }
            if (!optionValue->IsSoftwareSettable())
            {
                continue;
            }

            optionValue->Serialize(j[DeviceOptionsState::k_OptionsKey]);
        }
    }
}
