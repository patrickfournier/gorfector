#pragma once

#include <vector>
#include "StateComponent.hpp"

namespace ZooLib
{
    class Observer
    {
    protected:
        std::vector<const StateComponent *> m_ObservedComponents{};
        std::vector<StateComponent *> m_ModifiedComponents{};
        std::vector<uint64_t> m_ObservedComponentVersions{};

        virtual void UpdateImplementation() = 0;

    public:
        Observer(
                std::vector<const StateComponent *> observedComponents,
                std::vector<StateComponent *> modifiedComponents)
        {
            m_ObservedComponents = std::move(observedComponents);
            m_ModifiedComponents = std::move(modifiedComponents);
            m_ObservedComponentVersions = std::vector(m_ObservedComponents.size(), 0UL);
        }

        virtual ~Observer() = default;

        const std::vector<const StateComponent *> &ObservedComponents() const
        {
            return m_ObservedComponents;
        }

        const std::vector<StateComponent *> &ModifiedComponents() const
        {
            return m_ModifiedComponents;
        }

        void Update()
        {
            for (auto i = 0UL; i < m_ObservedComponents.size(); ++i)
            {
                const auto *observedComponent = m_ObservedComponents[i];
                auto &version = m_ObservedComponentVersions[i];
                if (observedComponent->GetVersion() != version)
                {
                    UpdateImplementation();
                    break;
                }
            }
            for (auto i = 0UL; i < m_ObservedComponents.size(); ++i)
            {
                const auto *observedComponent = m_ObservedComponents[i];
                auto &version = m_ObservedComponentVersions[i];
                version = observedComponent->GetVersion();
            }
        }
    };
}
