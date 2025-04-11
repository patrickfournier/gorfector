#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class AppState final : public ZooLib::StateComponent
    {
    public:
        enum AppMode
        {
            Single,
            Batch,
        };

        std::string m_OptionPanelDeviceName{};
        AppMode m_AppMode{};

        bool m_IsScanning{};
        bool m_IsPreviewing{};

    public:
        explicit AppState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        ~AppState() override
        {
        }

        [[nodiscard]] const std::string &GetOptionPanelDeviceName() const
        {
            return m_OptionPanelDeviceName;
        }

        [[nodiscard]] AppMode GetAppMode() const
        {
            return m_AppMode;
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

            void UpdateOptionPanelDeviceName(const std::string &deviceName) const
            {
                m_StateComponent->m_OptionPanelDeviceName = deviceName;
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
