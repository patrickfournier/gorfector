#pragma once

#include <vector>
#include "StateComponent.h"

namespace Zoo
{
    class Observer
    {
    protected:
        std::vector<Zoo::StateComponent *> m_ObservedComponents{};
        std::vector<Zoo::StateComponent *> m_ModifiedComponents{};

    public:
        Observer(std::vector<Zoo::StateComponent *> observedComponents,
                 std::vector<Zoo::StateComponent *> modifiedComponents)
        {
            m_ObservedComponents = std::move(observedComponents);
            m_ModifiedComponents = std::move(modifiedComponents);
        }

        virtual ~Observer() = default;

        virtual void Update() = 0;
    };
}
