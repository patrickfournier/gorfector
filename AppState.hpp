#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class AppState final : public ZooLib::StateComponent
    {
    public:
        enum ScanMode
        {
            Single,
            Batch,
        };

        std::string m_OptionPanelDeviceName{};
        ScanMode m_ScanMode{};

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

        [[nodiscard]] ScanMode GetScanMode() const
        {
            return m_ScanMode;
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

            void SetScanMode(ScanMode scanMode) const
            {
                m_StateComponent->m_ScanMode = scanMode;
            }
        };
    };
}
