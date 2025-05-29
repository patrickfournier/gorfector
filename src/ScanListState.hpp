#pragma once

#include <nlohmann/json.hpp>
#include <vector>

#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"

namespace Gorfector
{
    /**
     * \class ScanListStateChangeset
     * \brief Represents the changes made to the ScanListState.
     *
     * This class tracks modifications to the scan list, such as content changes,
     * selected item changes, or button action changes. It is used to efficiently
     * update the UI by only re-rendering the necessary parts.
     */
    class ScanListStateChangeset : public ZooLib::ChangesetBase
    {
    public:
        enum class TypeFlag
        {
            None = 0,
            ListContent = 1,
            SelectedItem = 2,
            ButtonAction = 4,
            ScanActivity = 8,
        };

    private:
        std::underlying_type_t<TypeFlag> m_ChangeType{};

    public:
        explicit ScanListStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        void Clear()
        {
            m_ChangeType = static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
        }

        void Set(TypeFlag typeFlag, int lastLine = -1)
        {
            m_ChangeType |= static_cast<std::underlying_type_t<TypeFlag>>(typeFlag);
        }

        [[nodiscard]] bool HasAnyChange() const
        {
            return m_ChangeType != static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
        }

        [[nodiscard]] bool IsChanged(TypeFlag typeFlag) const
        {
            return (m_ChangeType & static_cast<std::underlying_type_t<TypeFlag>>(typeFlag)) != 0;
        }

        void Aggregate(const ScanListStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ChangeType |= changeset.m_ChangeType;
        }
    };

    /**
     * \class ScanListState
     * \brief Manages the state of scan lists for different devices, including scan area, scanner settings, and output
     * settings.
     *
     * This class is responsible for maintaining and serializing scan lists for devices. It provides methods to
     * add, remove, and retrieve scan items, as well as manage the current device's scan list. The state is
     * serialized using `nlohmann::json` for persistence.
     */
    class ScanListState final : public ZooLib::StateComponent, public ZooLib::ChangesetManager<ScanListStateChangeset>
    {
    public:
        static constexpr const char *k_AddAllParamsKey = "AddAllParams";
        static constexpr const char *k_ScanListsKey = "ScanLists";

    private:
        static constexpr const char *k_ItemIdKey = "Id";
        static constexpr const char *k_ItemScanAreaHumanKey = "ScanAreaHuman";
        static constexpr const char *k_ItemScanAreaUnitsKey = "ScanAreaUnits";
        static constexpr const char *k_ItemScannerSettingsKey = "ScannerSettings";
        static constexpr const char *k_ItemOutputSettingsKey = "OutputSettings";
        static constexpr const char *k_ItemScanAreaSettingsKey = "ScanAreaSettings";

        std::map<std::string, nlohmann::json> m_ScanLists{};
        bool m_AddToScanListButtonAddsAllParams{};

        std::string m_CurrentDevice{};
        std::vector<nlohmann::json> m_CurrentScanList{};
        bool m_ScanActivity{};

        int m_SelectedIndex{-1};

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
                bool &isScanAreaItem) const
        {
            if (index < m_CurrentScanList.size())
            {
                id = m_CurrentScanList[index][k_ItemIdKey].get<int>();
                units = m_CurrentScanList[index][k_ItemScanAreaUnitsKey].get<std::string>();
                tlx = m_CurrentScanList[index][k_ItemScanAreaHumanKey][0].get<double>();
                tly = m_CurrentScanList[index][k_ItemScanAreaHumanKey][1].get<double>();
                brx = m_CurrentScanList[index][k_ItemScanAreaHumanKey][2].get<double>();
                bry = m_CurrentScanList[index][k_ItemScanAreaHumanKey][3].get<double>();

                isScanAreaItem = IsScanAreaItem(index);
            }
        }

        [[nodiscard]] bool IsScanAreaItem(size_t index) const
        {
            if (index < m_CurrentScanList.size())
            {
                return m_CurrentScanList[index].contains(k_ItemScanAreaSettingsKey);
            }
            return false;
        }

        [[nodiscard]] const nlohmann::json *GetScanAreaSettings(size_t index) const
        {
            if (index < m_CurrentScanList.size() && m_CurrentScanList[index].contains(k_ItemScanAreaSettingsKey))
            {
                return &m_CurrentScanList[index][k_ItemScanAreaSettingsKey];
            }
            return nullptr;
        }

        [[nodiscard]] const nlohmann::json *GetScannerSettings(size_t index) const
        {
            if (index < m_CurrentScanList.size() && m_CurrentScanList[index].contains(k_ItemScannerSettingsKey))
            {
                return &m_CurrentScanList[index][k_ItemScannerSettingsKey];
            }
            return nullptr;
        }

        [[nodiscard]] const nlohmann::json *GetOutputSettings(size_t index) const
        {
            if (index < m_CurrentScanList.size() && m_CurrentScanList[index].contains(k_ItemOutputSettingsKey))
            {
                return &m_CurrentScanList[index][k_ItemOutputSettingsKey];
            }
            return nullptr;
        }

        [[nodiscard]] bool GetAddToScanListButtonAddsAllParams() const
        {
            return m_AddToScanListButtonAddsAllParams;
        }

        [[nodiscard]] int GetSelectedIndex() const
        {
            return m_SelectedIndex;
        }

        [[nodiscard]] bool IsScanning() const
        {
            return m_ScanActivity;
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

            ~Updater() override
            {
                m_StateComponent->PushCurrentChangeset();
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
                LoadScanListForCurrentDevice();

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
            }

            void SetSelectedIndex(int index)
            {
                if (index >= 0 && index < static_cast<int>(m_StateComponent->m_CurrentScanList.size()))
                {
                    m_StateComponent->m_SelectedIndex = index;
                }
                else
                {
                    m_StateComponent->m_SelectedIndex = -1;
                }
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::SelectedItem);
            }

            void MoveScanListItem(size_t index, int positionDelta)
            {
                if (index >= m_StateComponent->m_CurrentScanList.size())
                {
                    return;
                }
                if (positionDelta == 0)
                {
                    return;
                }

                auto item = m_StateComponent->m_CurrentScanList[index];
                m_StateComponent->m_CurrentScanList.erase(
                        m_StateComponent->m_CurrentScanList.begin() + static_cast<int>(index));
                auto newIndex = static_cast<int>(index) + positionDelta;
                if (newIndex < 0)
                {
                    newIndex = 0;
                }
                else if (newIndex >= static_cast<int>(m_StateComponent->m_CurrentScanList.size()))
                {
                    newIndex = static_cast<int>(m_StateComponent->m_CurrentScanList.size());
                }
                m_StateComponent->m_CurrentScanList.insert(
                        m_StateComponent->m_CurrentScanList.begin() + newIndex, std::move(item));

                SetSelectedIndex(newIndex);

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
            }

            void AddCompleteScanItem(const DeviceOptionsState *deviceOptions, const OutputOptionsState *outputOptions)
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

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
            }

            void AddScanAreaItem(const DeviceOptionsState *deviceOptions)
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

                auto tlx = deviceOptions->GetOption<SANE_Word>(deviceOptions->GetTLXIndex());
                auto tly = deviceOptions->GetOption<SANE_Word>(deviceOptions->GetTLYIndex());
                auto brx = deviceOptions->GetOption<SANE_Word>(deviceOptions->GetBRXIndex());
                auto bry = deviceOptions->GetOption<SANE_Word>(deviceOptions->GetBRYIndex());
                // To simplify applying the settings, we store the values in the same format as the scan area preset.
                scanItem[k_ItemScanAreaSettingsKey] = {
                        {DeviceOptionsState::k_TlxKey, {tlx->GetValue()}},
                        {DeviceOptionsState::k_TlyKey, {tly->GetValue()}},
                        {DeviceOptionsState::k_BrxKey, {brx->GetValue()}},
                        {DeviceOptionsState::k_BryKey, {bry->GetValue()}},
                };
                m_StateComponent->m_CurrentScanList.emplace_back(std::move(scanItem));

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
            }

            void RemoveScanItemAt(size_t index)
            {
                if (index < m_StateComponent->m_CurrentScanList.size())
                {
                    m_StateComponent->m_CurrentScanList.erase(
                            m_StateComponent->m_CurrentScanList.begin() + static_cast<long>(index));

                    auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                    changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);

                    if (static_cast<int>(index) <= m_StateComponent->m_SelectedIndex)
                    {
                        if (static_cast<int>(index) == m_StateComponent->m_SelectedIndex)
                        {
                            m_StateComponent->m_SelectedIndex = -1;
                        }
                        else
                        {
                            m_StateComponent->m_SelectedIndex -= 1;
                        }
                    }
                    changeset->Set(ScanListStateChangeset::TypeFlag::SelectedItem);
                }
            }

            void ClearScanList()
            {
                m_StateComponent->m_CurrentScanList.clear();
                m_StateComponent->m_SelectedIndex = -1;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
                changeset->Set(ScanListStateChangeset::TypeFlag::SelectedItem);
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

                    auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                    changeset->Set(ScanListStateChangeset::TypeFlag::ListContent);
                }
            }

            void SetScanActivity(bool scanning)
            {
                m_StateComponent->m_ScanActivity = scanning;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ScanActivity);
            }

            void SetAddToScanListAddsAllParams(bool addsAllParams) const
            {
                m_StateComponent->m_AddToScanListButtonAddsAllParams = addsAllParams;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(ScanListStateChangeset::TypeFlag::ButtonAction);
            }
        };
    };

    inline void to_json(nlohmann::json &j, const ScanListState &state)
    {
        j = nlohmann::json{};
        j[ScanListState::k_AddAllParamsKey] = state.m_AddToScanListButtonAddsAllParams;
        j[ScanListState::k_ScanListsKey] = state.m_ScanLists;
    }

    inline void from_json(const nlohmann::json &j, ScanListState &state)
    {
        j.at(ScanListState::k_AddAllParamsKey).get_to(state.m_AddToScanListButtonAddsAllParams);
        j.at(ScanListState::k_ScanListsKey).get_to(state.m_ScanLists);
    }
}
