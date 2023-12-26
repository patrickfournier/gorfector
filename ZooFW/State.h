#pragma once

#include <vector>
#include <algorithm>

namespace Zoo
{
    class StateComponent;

    class State
    {
        std::vector<StateComponent *> m_StateComponents;

    public:
        void AddStateComponent(StateComponent *stateComponent)
        {
            // FIXME add once
            m_StateComponents.push_back(stateComponent);
        }

        void RemoveStateComponent(Zoo::StateComponent *stateComponent)
        {
            m_StateComponents.erase(std::remove(m_StateComponents.begin(), m_StateComponents.end(), stateComponent),
                                    m_StateComponents.end());
        }
    };
}
