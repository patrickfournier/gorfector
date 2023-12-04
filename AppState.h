#pragma once

#include <sane/sane.h>
#include "ZooFW/StateComponent.h"

namespace ZooScan
{
    class AppState : public Zoo::StateComponent
    {
        const SANE_Device *m_CurrentDevice{};

    public:
        [[nodiscard]] const SANE_Device *CurrentDevice() const
        { return m_CurrentDevice; }

        class Updater : public Zoo::StateComponent::Updater<AppState>
        {
        public:
            explicit Updater(AppState *state)
                    : StateComponent::Updater<AppState>(state)
            {}

            void SetCurrentDevice(const SANE_Device *device)
            {
                m_StateComponent->m_CurrentDevice = device;
            }
        };
    };
}
