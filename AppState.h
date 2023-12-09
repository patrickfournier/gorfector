#pragma once

#include <sane/sane.h>
#include "ZooFW/StateComponent.h"

namespace ZooScan
{
    class AppState : public Zoo::StateComponent
    {
        SaneDevice *m_CurrentDevice{};

    public:
        [[nodiscard]] const SaneDevice *CurrentDevice() const
        { return m_CurrentDevice; }

        class Updater : public Zoo::StateComponent::Updater<AppState>
        {
        public:
            explicit Updater(AppState *state)
                    : StateComponent::Updater<AppState>(state)
            {}

            void SetCurrentDevice(SaneDevice *device)
            {
                if (m_StateComponent->m_CurrentDevice != nullptr)
                    m_StateComponent->m_CurrentDevice->Close();

                m_StateComponent->m_CurrentDevice = device;

                if (m_StateComponent->m_CurrentDevice != nullptr)
                    m_StateComponent->m_CurrentDevice->Open();
            }
        };
    };
}
