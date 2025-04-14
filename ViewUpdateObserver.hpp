#pragma once

#include "ZooLib/Observer.hpp"

namespace ZooScan
{
    template<typename TView, typename... TStates>
    class ViewUpdateObserver : public ZooLib::Observer
    {
        TView *m_View{};

    protected:
        void UpdateImplementation() override
        {
            m_View->Update(m_ObservedComponentVersions);
        }

    public:
        explicit ViewUpdateObserver(TView *view, const TStates *...observedStateComponents)
            : Observer(
                      {
                              observedStateComponents...,
                      },
                      {})
        {
            m_View = view;
        }
    };
}
