#pragma once

#include <algorithm>
#include <vector>

namespace ZooLib
{
    class StateComponent;

    class State
    {
        std::vector<StateComponent *> m_StateComponents;

    public:
        void AddStateComponent(StateComponent *stateComponent)
        {
            if (std::ranges::find(m_StateComponents, stateComponent) == m_StateComponents.end())
            {
                m_StateComponents.push_back(stateComponent);
            }
        }

        void RemoveStateComponent(StateComponent *stateComponent)
        {
            if (const auto it = std::ranges::find(m_StateComponents, stateComponent); it != m_StateComponents.end())
            {
                m_StateComponents.erase(it);
            }
        }

        template <typename TStateComponent>
        TStateComponent* GetStateComponentByType() const
        {
            for (const auto &stateComponent : m_StateComponents)
            {
                auto sc = dynamic_cast<TStateComponent *>(stateComponent);
                if (sc != nullptr)
                {
                    return sc;
                }
            }

            return nullptr;
        }
    };
}
