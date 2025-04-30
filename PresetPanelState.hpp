#pragma once

#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class PresetPanelState : public ZooLib::StateComponent
    {
    public:
        static const char *s_PresetSectionScanArea;
        static const char *s_PresetSectionScannerSettings;
        static const char *s_PresetSectionOutputSettings;

    private:
        bool m_Expanded{};
        std::vector<nlohmann::json> m_Presets{};

        std::string m_CurrentDeviceModel{};
        std::string m_CurrentDeviceVendor{};

        friend void to_json(nlohmann::json &j, const PresetPanelState &state);
        friend void from_json(const nlohmann::json &j, PresetPanelState &state);

    public:
        explicit PresetPanelState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        ~PresetPanelState() override
        {
            m_State->SaveToFile(this);
        }

        [[nodiscard]] const std::string &GetCurrentDeviceVendor() const
        {
            return m_CurrentDeviceVendor;
        }

        [[nodiscard]] const std::string &GetCurrentDeviceModel() const
        {
            return m_CurrentDeviceModel;
        }

        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "PresetPanelState";
        }

        [[nodiscard]] bool CanCreateOrApplyPreset() const
        {
            return !m_CurrentDeviceModel.empty() && !m_CurrentDeviceVendor.empty();
        }

        [[nodiscard]] bool IsExpanded() const
        {
            return m_Expanded;
        }

        [[nodiscard]] std::vector<nlohmann::json>
        GetPresetsForScanner(const std::string &vendorName, const std::string &modelName)
        {
            auto results = std::vector<nlohmann::json>{};
            for (auto preset: m_Presets)
            {
                if (preset.contains("ScannerSettings"))

                {
                    if (preset["ScannerSettings"]["Device"]["Vendor"] == vendorName &&
                        preset["ScannerSettings"]["Device"]["Model"] == modelName)
                    {
                        results.push_back(preset);
                    }
                }
                else
                {
                    results.push_back(preset);
                }
            }

            return results;
        }

        [[nodiscard]] const nlohmann::json *GetPreset(const std::string &presetId) const
        {
            auto it = std::ranges::find_if(
                    m_Presets, [&presetId](const nlohmann::json &preset) { return preset["Name"] == presetId; });
            if (it != m_Presets.end())
            {
                return &*it;
            }

            return nullptr;
        }

        class Updater final : public StateComponent::Updater<PresetPanelState>
        {
        public:
            explicit Updater(PresetPanelState *state)
                : StateComponent::Updater<PresetPanelState>(state)
            {
            }

            void SetCurrentDeviceName(const std::string &vendorName, const std::string &modelName)
            {
                m_StateComponent->m_CurrentDeviceVendor = vendorName;
                m_StateComponent->m_CurrentDeviceModel = modelName;
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            void SetExpanded(bool expanded)
            {
                if (m_StateComponent->m_Expanded == expanded)
                {
                    return;
                }

                m_StateComponent->m_Expanded = expanded;
            }

            void AddPreset(const nlohmann::json &preset)
            {
                m_StateComponent->m_Presets.push_back(preset);
            }

            void RemovePreset(const std::string &uniqueId)
            {
                m_StateComponent->m_Presets.erase(
                        std::ranges::remove_if(
                                m_StateComponent->m_Presets,
                                [&uniqueId](const nlohmann::json &preset) { return preset["Name"] == uniqueId; })
                                .begin(),
                        m_StateComponent->m_Presets.end());
            }

            void RenamePreset(std::string presetId, const std::string &newName)
            {
                auto it = std::ranges::find_if(m_StateComponent->m_Presets, [&presetId](const nlohmann::json &preset) {
                    return preset["Name"] == presetId;
                });
                if (it != m_StateComponent->m_Presets.end())
                {
                    (*it)["Name"] = newName;
                }
            }
        };
    };

    inline void to_json(nlohmann::json &j, const PresetPanelState &state)
    {
        j = nlohmann::json{{"Expanded", state.m_Expanded}, {"Presets", state.m_Presets}};
    }

    inline void from_json(const nlohmann::json &j, PresetPanelState &state)
    {
        j.at("Expanded").get_to(state.m_Expanded);
        j.at("Presets").get_to(state.m_Presets);
    }
}
