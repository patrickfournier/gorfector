#pragma once

#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class PngWriterStateComponent : public ZooLib::StateComponent
    {
        int m_CompressionLevel{};

    public:
        explicit PngWriterStateComponent(ZooLib::State *state)
            : StateComponent(state)
            , m_CompressionLevel(1)
        {
        }

        [[nodiscard]] int GetCompressionLevel() const
        {
            return m_CompressionLevel;
        }
    };
}
