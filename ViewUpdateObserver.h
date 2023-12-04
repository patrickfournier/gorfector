#pragma once

#include "ZooFW/Observer.h"

namespace ZooScan
{
    template<typename TView, typename TState>
    class ViewUpdateObserver : public Zoo::Observer
    {
        TView *m_View{};

    public:
        ViewUpdateObserver(TView *view,
                           TState *observedStateComponent)
        : Observer({observedStateComponent}, {})
        {
            m_View = view;
        }

        void Update() override
        {
            m_View->Update();
        }
    };
}
