#pragma once

#include <cstring>
#include <stdexcept>
#include "ZooFW/StateComponent.hpp"
#include "DeviceOptionValueBase.hpp"
#include "DeviceOptionValue.hpp"
#include "ZooFW/ChangesetManager.hpp"
#include "ZooFW/ChangesetBase.hpp"
#include "nlohmann_json/json.hpp"

namespace ZooScan
{
    struct WidgetIndex
    {
        const uint32_t m_OptionIndex{};
        const uint32_t m_ValueIndex{};

        WidgetIndex() = default;
        WidgetIndex(uint32_t optionIndex, uint32_t valueIndex)
                : m_OptionIndex(optionIndex), m_ValueIndex(valueIndex)
        {
        }
        explicit WidgetIndex(uint64_t hash)
                : m_OptionIndex(uint32_t(hash >> 32)), m_ValueIndex(uint32_t(hash))
        {
        }
        WidgetIndex(const WidgetIndex &other) = default;
        WidgetIndex(WidgetIndex &&other) noexcept = default;

        bool operator==(const WidgetIndex &other) const
        {
            return m_OptionIndex == other.m_OptionIndex && m_ValueIndex == other.m_ValueIndex;
        }

        [[nodiscard]] uint64_t Hash() const
        {
            return (uint64_t(m_OptionIndex) << 32) | m_ValueIndex;
        }
    };

    class Changeset : public Zoo::ChangesetBase
    {
        std::vector<WidgetIndex> m_ChangedIndices;
        bool m_ReloadOptions{};

    public:
        explicit Changeset(uint64_t stateInitialVersion)
                : ChangesetBase(stateInitialVersion)
        {
        }

        void AddChangedIndex(WidgetIndex index)
        {
            m_ChangedIndices.push_back(index);
        }

        [[nodiscard]] const std::vector<WidgetIndex> &ChangedIndices() const
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

        void Aggregate(const Changeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ReloadOptions |= changeset.m_ReloadOptions;

            for (auto changedIndex: changeset.m_ChangedIndices)
            {
                if (std::find(m_ChangedIndices.begin(), m_ChangedIndices.end(), changedIndex) == m_ChangedIndices.end())
                {
                    m_ChangedIndices.push_back(changedIndex);
                }
            }
        }
    };

    class SaneDevice;

    class DeviceOptionState : public Zoo::StateComponent, public Zoo::ChangesetManager<Changeset>
    {
        SaneDevice* m_Device;
        std::vector<DeviceOptionValueBase *> m_OptionValues;

        uint32_t m_PreviewIndex{};
        uint32_t m_ModeIndex{};
        uint32_t m_TLXIndex{};
        uint32_t m_TLYIndex{};
        uint32_t m_BRXIndex{};
        uint32_t m_BRYIndex{};
        uint32_t m_ResolutionIndex{};
        uint32_t m_XResolutionIndex{};
        uint32_t m_YResolutionIndex{};
        uint32_t m_BitDepthIndex{};

        [[nodiscard]] Changeset *GetCurrentChangeset()
        {
            return ChangesetManager::GetCurrentChangeset(Version());
        }

    public:
        DeviceOptionState(Zoo::State* state, SaneDevice* device)
        : Zoo::StateComponent(state)
        , m_Device(device)
        {
        }

        ~DeviceOptionState() override
        {
            for (auto optionValue: m_OptionValues)
            {
                delete optionValue;
            }
        }

        class Updater : public Zoo::StateComponent::Updater<DeviceOptionState>
        {
        public:
            explicit Updater(DeviceOptionState *state)
                    : Zoo::StateComponent::Updater<DeviceOptionState>(state)
            {}

            ~Updater() override
            {
                m_StateComponent->PushCurrentChangeset();
            }

            void BuildOptions();

            void AddOptionValue(uint32_t index, DeviceOptionValueBase *optionValue)
            {
                if (m_StateComponent->m_OptionValues.size() <= index)
                {
                    m_StateComponent->m_OptionValues.resize(index + 1);
                }
                m_StateComponent->m_OptionValues[index] = optionValue;

                /* Save special indices */

                if (strcmp(optionValue->Name(), "preview") == 0)
                {
                    m_StateComponent->m_PreviewIndex = index;
                }
                else if (strcmp(optionValue->Name(), "mode") == 0)  // not a "well-known" option
                {
                    m_StateComponent->m_ModeIndex = index;
                }
                else if (strcmp(optionValue->Name(), "tl-x") == 0)
                {
                    m_StateComponent->m_TLXIndex = index;
                }
                else if (strcmp(optionValue->Name(), "tl-y") == 0)
                {
                    m_StateComponent->m_TLYIndex = index;
                }
                else if (strcmp(optionValue->Name(), "br-x") == 0)
                {
                    m_StateComponent->m_BRXIndex = index;
                }
                else if (strcmp(optionValue->Name(), "br-y") == 0)
                {
                    m_StateComponent->m_BRYIndex = index;
                }
                else if (strcmp(optionValue->Name(), "resolution") == 0)
                {
                    m_StateComponent->m_ResolutionIndex = index;
                }
                else if (strcmp(optionValue->Name(), "x-resolution") == 0)  // not a "well-known" option
                {
                    m_StateComponent->m_XResolutionIndex = index;
                }
                else if (strcmp(optionValue->Name(), "y-resolution") == 0)  // not a "well-known" option
                {
                    m_StateComponent->m_YResolutionIndex = index;
                }
                else if (strcmp(optionValue->Name(), "depth") == 0)  // not a "well-known" option
                {
                    m_StateComponent->m_BitDepthIndex = index;
                }
            }

            void SetOption(uint32_t optionIndex, uint32_t valueIndex, bool requestedValue);
            void SetOption(uint32_t optionIndex, uint32_t valueIndex, int requestedValue);
            void SetOption(uint32_t optionIndex, uint32_t valueIndex, const std::string& requestedValue);

            void Clear()
            {
                for (auto optionValue: m_StateComponent->m_OptionValues)
                {
                    delete optionValue;
                }
                m_StateComponent->m_OptionValues.clear();
                m_StateComponent->GetCurrentChangeset()->SetReloadOptions(true);
            }

            void DeserializeAndApply(const nlohmann::json& json);
        };

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

        [[nodiscard]] double GetScanAreaX() const;

        [[nodiscard]] double GetScanAreaY() const;

        [[nodiscard]] double GetScanAreaWidth() const;

        [[nodiscard]] double GetScanAreaHeight() const;

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

        [[nodiscard]] nlohmann::json* Serialize() const;
    };
}
