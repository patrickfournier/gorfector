#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{
    class JpegWriterStateComponent : public ZooLib::StateComponent
    {
        int m_Quality;

    public:
        explicit JpegWriterStateComponent(ZooLib::State *state)
            : StateComponent(state)
            , m_Quality(75)
        {
        }

        int GetQuality() const
        {
            return m_Quality;
        }
    };
}
