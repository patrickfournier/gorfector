#pragma once

#include <sane/sane.h>
#include "ZooFW/StateComponent.h"

namespace ZooScan
{
    class AppState : public Zoo::StateComponent
    {
        SaneDevice *m_CurrentDevice{};
        DeviceOptionState *m_CurrentDeviceOptions{};

    public:
        explicit AppState(Zoo::State* state)
                : StateComponent(state)
        {}

        ~AppState() override
        {
            if (m_CurrentDevice != nullptr)
                m_CurrentDevice->Close();
        }

        [[nodiscard]] const SaneDevice *CurrentDevice() const
        { return m_CurrentDevice; }

        [[nodiscard]] DeviceOptionState *DeviceOptions()
        { return m_CurrentDeviceOptions; }

        class Updater : public Zoo::StateComponent::Updater<AppState>
        {
        public:
            explicit Updater(AppState *state)
                    : StateComponent::Updater<AppState>(state)
            {}

            void SetCurrentDevice(SaneDevice *device)
            {
                if (m_StateComponent->m_CurrentDevice == device)
                    return;

                delete m_StateComponent->m_CurrentDeviceOptions;

                if (m_StateComponent->m_CurrentDevice != nullptr)
                    m_StateComponent->m_CurrentDevice->Close();

                m_StateComponent->m_CurrentDevice = device;

                if (m_StateComponent->m_CurrentDevice != nullptr)
                {
                    m_StateComponent->m_CurrentDevice->Open();

                    m_StateComponent->m_CurrentDeviceOptions = new DeviceOptionState(m_StateComponent->m_State, m_StateComponent->m_CurrentDevice);
                    auto optionUpdater = DeviceOptionState::Updater(m_StateComponent->m_CurrentDeviceOptions);
                    optionUpdater.BuildOptions();
                }
            }
        };
    };
}
