#pragma once

#include "StateComponent.h"
#include <vector>
#include <algorithm>

namespace Zoo
{
    class State
    {
        std::vector<StateComponent *> m_StateComponents;

    public:
        void AddStateComponent(StateComponent *stateComponent)
        {
            m_StateComponents.push_back(stateComponent);
        }

        void RemoveStateComponent(Zoo::StateComponent *stateComponent)
        {
            m_StateComponents.erase(std::remove(m_StateComponents.begin(), m_StateComponents.end(), stateComponent),
                                    m_StateComponents.end());
        }
    };
}
