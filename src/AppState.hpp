#pragma once

#include "ScanListState.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class AppStateChangeset : public ZooLib::ChangesetBase
    {
    public:
        enum class ChangeTypeFlag
        {
            e_PaneSplitter = 1 << 0,
            e_CurrentDevice = 1 << 1,
            e_ScanActivity = 1 << 2,
            e_ScanListMode = 1 << 3,
        };

    private:
        int m_ChangeType{};

    public:
        explicit AppStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        void AddChangeType(ChangeTypeFlag changeType)
        {
            m_ChangeType |= static_cast<int>(changeType);
        }

        [[nodiscard]] bool IsChanged(ChangeTypeFlag changeType) const
        {
            return (m_ChangeType & static_cast<int>(changeType)) != 0;
        }

        [[nodiscard]] bool HasAnyChange() const
        {
            return m_ChangeType != 0;
        }

        void Aggregate(const AppStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ChangeType |= changeset.m_ChangeType;
        }
    };

    class AppState final : public ZooLib::StateComponent
    {
    public:
        static constexpr const char *k_UseScanListKey = "UseScanList";
        static constexpr const char *k_LeftPanelWidthKey = "LeftPanelWidth";
        static constexpr const char *k_RightPanelWidthKey = "RightPanelWidth";

    private:
        const bool m_DevMode;
        bool m_UseScanList{false};
        double m_LeftPanelWidth{.3};
        double m_RightPanelWidth{.3};

        std::string m_CurrentDeviceName{};

        bool m_IsScanning{};
        bool m_IsPreviewing{};

        ZooLib::ChangesetManager<AppStateChangeset> m_ChangesetManager{};

        [[nodiscard]] AppStateChangeset *GetCurrentChangeset()
        {
            return m_ChangesetManager.GetCurrentChangeset(GetVersion());
        }

        friend void to_json(nlohmann::json &j, const AppState &state);
        friend void from_json(const nlohmann::json &j, AppState &state);

    public:
        explicit AppState(ZooLib::State *state, bool devMode)
            : StateComponent(state)
            , m_DevMode(devMode)
        {
        }

        ~AppState() override
        {
            m_State->SaveToFile(this);
        }

        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "AppState";
        }

        [[nodiscard]] const std::string &GetCurrentDeviceName() const
        {
            return m_CurrentDeviceName;
        }

        [[nodiscard]] bool IsDeveloperMode() const
        {
            return m_DevMode;
        }

        [[nodiscard]] bool GetUseScanList() const
        {
            return m_UseScanList;
        }

        [[nodiscard]] double GetLeftPanelWidth() const
        {
            return m_LeftPanelWidth;
        }

        [[nodiscard]] double GetRightPanelWidth() const
        {
            return m_RightPanelWidth;
        }

        [[nodiscard]] bool IsScanning() const
        {
            return m_IsScanning;
        }

        [[nodiscard]] bool IsPreviewing() const
        {
            return m_IsPreviewing;
        }

        [[nodiscard]] ZooLib::ChangesetManagerBase *GetChangesetManager() override
        {
            return &m_ChangesetManager;
        }

        [[nodiscard]] AppStateChangeset *GetAggregatedChangeset(uint64_t stateComponentVersion) const
        {
            return m_ChangesetManager.GetAggregatedChangeset(stateComponentVersion);
        }

        class Updater final : public StateComponent::Updater<AppState>
        {
        public:
            explicit Updater(AppState *state)
                : StateComponent::Updater<AppState>(state)
            {
            }

            ~Updater() override
            {
                m_StateComponent->m_ChangesetManager.PushCurrentChangeset();
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_ScanListMode);
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_PaneSplitter);
            }

            void SetUseScanList(bool useScanList) const
            {
                m_StateComponent->m_UseScanList = useScanList;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_ScanListMode);
            }

            void SetLeftPanelWidth(double width) const
            {
                m_StateComponent->m_LeftPanelWidth = width;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_PaneSplitter);
            }

            void SetRightPanelWidth(double width) const
            {
                m_StateComponent->m_RightPanelWidth = width;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_PaneSplitter);
            }

            void SetCurrentDevice(const std::string &deviceName) const
            {
                m_StateComponent->m_CurrentDeviceName = deviceName;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_CurrentDevice);
            }

            void SetIsScanning(bool isScanning) const
            {
                m_StateComponent->m_IsScanning = isScanning;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_ScanActivity);
            }

            void SetIsPreviewing(bool isPreviewing) const
            {
                m_StateComponent->m_IsPreviewing = isPreviewing;
                m_StateComponent->GetCurrentChangeset()->AddChangeType(
                        AppStateChangeset::ChangeTypeFlag::e_ScanActivity);
            }
        };
    };

    inline void to_json(nlohmann::json &j, const AppState &state)
    {
        j = {
                {AppState::k_UseScanListKey, state.m_UseScanList},
                {AppState::k_LeftPanelWidthKey, state.m_LeftPanelWidth},
                {AppState::k_RightPanelWidthKey, state.m_RightPanelWidth},
        };
    }

    inline void from_json(const nlohmann::json &j, AppState &state)
    {
        state.m_UseScanList = j.value(AppState::k_UseScanListKey, false);
        state.m_LeftPanelWidth = j.value(AppState::k_LeftPanelWidthKey, 0.3);
        state.m_RightPanelWidth = j.value(AppState::k_RightPanelWidthKey, 0.3);
    }
}
