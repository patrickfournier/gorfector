#pragma once

#include "ZooFW/Observer.hpp"

namespace ZooScan
{
    template<typename TView, typename TState>
    class ViewUpdateObserver : public Zoo::Observer
    {
        TView *m_View{};

    public:
        ViewUpdateObserver(TView *view,
                           const TState *observedStateComponent)
        : Observer({observedStateComponent}, {})
        {
            m_View = view;
        }

        void Update() override
        {
            m_View->Update(dynamic_cast<const TState *>(m_ObservedComponents[0]));
        }
    };
}
