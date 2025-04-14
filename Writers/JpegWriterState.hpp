#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class JpegWriterState : public ZooLib::StateComponent
    {
        int m_Quality;

    public:
        explicit JpegWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_Quality(75)
        {
        }

        int GetQuality() const
        {
            return m_Quality;
        }

        class Updater final : public StateComponent::Updater<JpegWriterState>
        {
        public:
            explicit Updater(JpegWriterState *state)
                : StateComponent::Updater<JpegWriterState>(state)
            {
            }

            void SetQuality(int quality)
            {
                m_StateComponent->m_Quality = quality;
            }
        };
    };
}
