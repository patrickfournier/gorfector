#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class PngWriterState : public ZooLib::StateComponent
    {
        int m_CompressionLevel{};

    public:
        explicit PngWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_CompressionLevel(1)
        {
        }

        [[nodiscard]] int GetCompressionLevel() const
        {
            return m_CompressionLevel;
        }

        class Updater final : public StateComponent::Updater<PngWriterState>
        {
        public:
            explicit Updater(PngWriterState *state)
                : StateComponent::Updater<PngWriterState>(state)
            {
            }

            void SetCompressionLevel(int compressionLevel)
            {
                m_StateComponent->m_CompressionLevel = compressionLevel;
            }
        };
    };
}
