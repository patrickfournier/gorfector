#pragma once

#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class AppState final : public ZooLib::StateComponent
    {
    public:
        enum AppMode
        {
            Single,
            Batch,
        };

    private:
        const bool m_DevMode;

        std::string m_CurrentDeviceName{};
        AppMode m_AppMode{};

        bool m_IsScanning{};
        bool m_IsPreviewing{};

    public:
        explicit AppState(ZooLib::State *state, bool devMode)
            : StateComponent(state)
            , m_DevMode(devMode)
        {
        }

        ~AppState() override = default;

        [[nodiscard]] const std::string &GetCurrentDeviceName() const
        {
            return m_CurrentDeviceName;
        }

        [[nodiscard]] AppMode GetAppMode() const
        {
            return m_AppMode;
        }

        [[nodiscard]] bool IsDeveloperMode() const
        {
            return m_DevMode;
        }

        [[nodiscard]] bool IsScanning() const
        {
            return m_IsScanning;
        }

        [[nodiscard]] bool IsPreviewing() const
        {
            return m_IsPreviewing;
        }

        class Updater final : public StateComponent::Updater<AppState>
        {
        public:
            explicit Updater(AppState *state)
                : StateComponent::Updater<AppState>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
            }

            void SetCurrentDevice(const std::string &deviceName) const
            {
                m_StateComponent->m_CurrentDeviceName = deviceName;
            }

            void SetAppMode(AppMode scanMode) const
            {
                m_StateComponent->m_AppMode = scanMode;
            }

            void SetIsScanning(bool isScanning) const
            {
                m_StateComponent->m_IsScanning = isScanning;
            }

            void SetIsPreviewing(bool isPreviewing) const
            {
                m_StateComponent->m_IsPreviewing = isPreviewing;
            }
        };
    };
}
