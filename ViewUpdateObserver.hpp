#pragma once

#include "ZooLib/Observer.hpp"

namespace ZooScan
{
    template<typename TView, typename TState>
    class ViewUpdateObserver : public ZooLib::Observer
    {
        TView *m_View{};

    protected:
        void UpdateImplementation() override
        {
            m_View->Update(m_ObservedComponentVersions[0]);
        }

    public:
        ViewUpdateObserver(TView *view, const TState *observedStateComponent)
            : Observer({observedStateComponent}, {})
        {
            m_View = view;
        }
    };
}
