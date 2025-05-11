#pragma once

#include <gtk/gtk.h>
#include <vector>

#include <nlohmann/json.hpp>
#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"

namespace Gorfector
{
    class ScanListState final : public ZooLib::StateComponent
    {
        static constexpr const char *k_ItemIdKey = "Id";
        static constexpr const char *k_ItemScanAreaHumanKey = "ScanAreaHuman";
        static constexpr const char *k_ItemScanAreaUnitsKey = "ScanAreaUnits";
        static constexpr const char *k_ItemScannerSettingsKey = "ScannerSettings";
        static constexpr const char *k_ItemOutputSettingsKey = "OutputSettings";

        std::map<std::string, nlohmann::json> m_ScanLists{};

        std::string m_CurrentDevice{};
        std::vector<nlohmann::json> m_CurrentScanList{};

        friend void to_json(nlohmann::json &j, const ScanListState &state);
        friend void from_json(const nlohmann::json &j, ScanListState &state);

    public:
        explicit ScanListState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        ~ScanListState() override
        {
            if (!m_CurrentDevice.empty())
            {
                m_ScanLists[m_CurrentDevice] = m_CurrentScanList;
            }
            m_State->SaveToFile(this);
        }

        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "ScanListState";
        }

        [[nodiscard]] size_t GetScanListSize() const
        {
            return m_CurrentScanList.size();
        }

        [[nodiscard]] int GetScanItemId(size_t index) const
        {
            if (index < m_CurrentScanList.size())
            {
                return m_CurrentScanList[index][k_ItemIdKey].get<int>();
            }
            return -1;
        }

        void GetScanItemInfos(
                size_t index, int &id, std::string &units, double &tlx, double &tly, double &brx, double &bry,
                std::string &outputFilePath) const
        {
            if (index < m_CurrentScanList.size())
            {
                id = m_CurrentScanList[index][k_ItemIdKey].get<int>();
                units = m_CurrentScanList[index][k_ItemScanAreaUnitsKey].get<std::string>();
                tlx = m_CurrentScanList[index][k_ItemScanAreaHumanKey][0].get<double>();
                tly = m_CurrentScanList[index][k_ItemScanAreaHumanKey][1].get<double>();
                brx = m_CurrentScanList[index][k_ItemScanAreaHumanKey][2].get<double>();
                bry = m_CurrentScanList[index][k_ItemScanAreaHumanKey][3].get<double>();

                if (m_CurrentScanList[index][k_ItemOutputSettingsKey][OutputOptionsState::k_OutputDestinationKey] ==
                    OutputOptionsState::OutputDestination::e_File)
                {
                    std::filesystem::path outputFilePathObj =
                            m_CurrentScanList[index][k_ItemOutputSettingsKey][OutputOptionsState::k_OutputDirectoryKey]
                                    .get<std::string>();
                    outputFilePathObj /=
                            m_CurrentScanList[index][k_ItemOutputSettingsKey][OutputOptionsState::k_OutputFileNameKey]
                                    .get<std::string>();
                    outputFilePath = outputFilePathObj.string();
                }
                else if (
                        m_CurrentScanList[index][k_ItemOutputSettingsKey][OutputOptionsState::k_OutputDestinationKey] ==
                        OutputOptionsState::OutputDestination::e_Printer)
                {
                    outputFilePath = _("<Printer>");
                }
                else if (
                        m_CurrentScanList[index][k_ItemOutputSettingsKey][OutputOptionsState::k_OutputDestinationKey] ==
                        OutputOptionsState::OutputDestination::e_Email)
                {
                    outputFilePath = _("<Email>");
                }
                else
                {
                    outputFilePath = "";
                }
            }
        }

        [[nodiscard]] const nlohmann::json *GetScannerSettings(size_t index) const
        {
            if (index < m_CurrentScanList.size())
            {
                return &m_CurrentScanList[index][k_ItemScannerSettingsKey];
            }
            return nullptr;
        }

        [[nodiscard]] const nlohmann::json *GetOutputSettings(size_t index) const
        {
            if (index < m_CurrentScanList.size())
            {
                return &m_CurrentScanList[index][k_ItemOutputSettingsKey];
            }
            return nullptr;
        }

        class Updater final : public StateComponent::Updater<ScanListState>
        {
            void LoadScanListForCurrentDevice()
            {
                if (m_StateComponent->m_ScanLists.contains(m_StateComponent->m_CurrentDevice))
                {
                    m_StateComponent->m_CurrentScanList =
                            m_StateComponent->m_ScanLists[m_StateComponent->m_CurrentDevice];
                }
                else
                {
                    m_StateComponent->m_CurrentScanList = {};
                }
            }

        public:
            explicit Updater(ScanListState *state)
                : StateComponent::Updater<ScanListState>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
                LoadScanListForCurrentDevice();
            }

            void AddScanItem(const DeviceOptionsState *deviceOptions, const OutputOptionsState *outputOptions)
            {
                auto itemId = 1;
                if (!m_StateComponent->m_CurrentScanList.empty())
                {
                    auto lastItem = m_StateComponent->m_CurrentScanList.back();
                    itemId = lastItem[k_ItemIdKey].get<int>() + 1;
                }

                std::string scanAreaUnits =
                        deviceOptions->GetScanAreaUnit() == ScanAreaUnit::e_Millimeters ? "mm" : "px";
                auto scanAreaHuman = deviceOptions->GetScanArea();
                auto scanItem = nlohmann::json{
                        {k_ItemIdKey, itemId},
                        {k_ItemScanAreaUnitsKey, scanAreaUnits},
                        {k_ItemScanAreaHumanKey,
                         {scanAreaHuman.MinX(), scanAreaHuman.MinY(), scanAreaHuman.MaxX(), scanAreaHuman.MaxY()}},
                };
                to_json(scanItem[k_ItemScannerSettingsKey], *deviceOptions);
                to_json(scanItem[k_ItemOutputSettingsKey], *outputOptions);
                m_StateComponent->m_CurrentScanList.emplace_back(std::move(scanItem));
            }

            void RemoveScanItemAt(size_t index)
            {
                if (index < m_StateComponent->m_CurrentScanList.size())
                {
                    m_StateComponent->m_CurrentScanList.erase(m_StateComponent->m_CurrentScanList.begin() + index);
                }
            }

            void ClearScanList()
            {
                m_StateComponent->m_CurrentScanList.clear();
            }

            void SetCurrentDeviceName(const std::string &vendorName, const std::string &modelName)
            {
                auto newDeviceName = vendorName.empty() ? "" : vendorName + "::" + modelName;
                if (newDeviceName != m_StateComponent->m_CurrentDevice)
                {
                    if (!m_StateComponent->m_CurrentDevice.empty())
                    {
                        m_StateComponent->m_ScanLists[m_StateComponent->m_CurrentDevice] =
                                m_StateComponent->m_CurrentScanList;
                    }

                    m_StateComponent->m_CurrentDevice = newDeviceName;
                    LoadScanListForCurrentDevice();
                }
            }
        };
    };

    inline void to_json(nlohmann::json &j, const ScanListState &state)
    {
        to_json(j, state.m_ScanLists);
    }

    inline void from_json(const nlohmann::json &j, ScanListState &state)
    {
        from_json(j, state.m_ScanLists);
    }
}
