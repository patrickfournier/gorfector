#pragma once

#include <vector>

#include "StateComponent.hpp"

namespace ZooLib
{
    /**
     * \class Observer
     * \brief A class responsible for observing changes in state components and responding to updates.
     *
     * The Observer class monitors a set of state components and executes `UpdateImplementation()` when a change is
     * detected. It is a base class for creating observers that can respond to changes in state components. If the
     * observer modifies any state components, they should be passed in the constructor to ensure that observers are
     * executed in the correct order.
     */
    class Observer
    {
    protected:
        /**
         * \brief The state components being observed for changes.
         */
        std::vector<const StateComponent *> m_ObservedComponents{};

        /**
         * \brief The state components that may be modified by the observer.
         */
        std::vector<StateComponent *> m_ModifiedComponents{};

        /**
         * \brief The versions of the observed state components, when they were last observed.
         */
        std::vector<uint64_t> m_ObservedComponentVersions{};

        /**
         * \brief Pure virtual method to be implemented by derived classes to handle updates.
         *
         * This method is called when a change is detected in the observed state components.
         */
        virtual void UpdateImplementation() = 0;

    public:
        /**
         * \brief Constructs an Observer with the specified observed and modified components.
         *
         * \param observedComponents The state components to observe for changes.
         * \param modifiedComponents The state components that may be modified by the observer.
         */
        Observer(
                std::vector<const StateComponent *> observedComponents,
                std::vector<StateComponent *> modifiedComponents)
        {
            m_ObservedComponents = std::move(observedComponents);
            m_ModifiedComponents = std::move(modifiedComponents);
            m_ObservedComponentVersions = std::vector(m_ObservedComponents.size(), 0UL);
        }

        /**
         * \brief Virtual destructor for the Observer class.
         */
        virtual ~Observer() = default;

        /**
         * \brief Retrieves the list of observed state components.
         * \return A constant reference to the vector of observed state components.
         */
        [[nodiscard]] const std::vector<const StateComponent *> &GetObservedComponents() const
        {
            return m_ObservedComponents;
        }

        /**
         * \brief Retrieves the list of modified state components.
         * \return A constant reference to the vector of modified state components.
         */
        [[nodiscard]] const std::vector<StateComponent *> &GetModifiedComponents() const
        {
            return m_ModifiedComponents;
        }

        /**
         * \brief Checks for changes in the observed components and updates their versions.
         *
         * This method iterates through the observed components and compares their current versions
         * with the stored versions. If a change is detected, `UpdateImplementation()` is called.
         * Afterward, the stored versions are updated to match the current versions.
         */
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
