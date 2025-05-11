#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "Observer.hpp"

namespace ZooLib
{
    /**
     * \class ObserverManager
     * \brief Manages a collection of observers, ensuring they are sorted and notified in the correct order.
     *
     * The ObserverManager handles the addition, removal, and notification of observers. It ensures that
     * observers are executed in a topologically sorted order based on their dependencies.
     */
    class ObserverManager
    {
        std::vector<Observer *> m_Observers{}; ///< List of all observers.
        bool m_NeedsSorting{}; ///< Flag indicating if the observers need to be sorted.
        std::vector<Observer *> m_SortedObservers{}; ///< List of observers sorted by dependency.
        std::vector<Observer *> m_DeletedObservers{}; ///< List of observers marked for deletion.

        /**
         * \brief Removes an item from a vector by swapping it with the last element and popping it.
         * \tparam T The type of the elements in the vector.
         * \param vec The vector from which the item will be removed.
         * \param item The item to remove.
         */
        template<typename T>
        void SwapRemove(std::vector<T *> &vec, T *item)
        {
            if (vec.empty())
                return;

            if (vec[vec.size() - 1] == item)
            {
                vec.pop_back();
                return;
            }

            auto it = std::ranges::find(vec, item);
            if (it != vec.end())
            {
                std::iter_swap(it, vec.end() - 1);
                vec.pop_back();
            }
        }

        /**
         * \brief Sorts the observers in topological order based on their dependencies.
         *
         * Observers form a directed graph based on their observed and modified state components.
         * This method performs a topological sort to ensure the correct execution order.
         *
         * \throws std::runtime_error If the sorting fails due to cyclic dependencies.
         */
        void SortObservers()
        {
            // See https://en.wikipedia.org/wiki/Topological_sorting.

            m_SortedObservers.clear();
            std::vector observersToSort(m_Observers);
            std::vector<const StateComponent *> modifiedComponents;
            for (const Observer *observer: observersToSort)
            {
                for (auto modifiedSC: observer->GetModifiedComponents())
                {
                    modifiedComponents.push_back(modifiedSC);
                }
            }

            bool nextObserverFound = true;
            while (!observersToSort.empty() && nextObserverFound)
            {
                nextObserverFound = false;
                for (Observer *observer: observersToSort)
                {
                    if (std::ranges::all_of(
                                observer->GetObservedComponents(),
                                [&modifiedComponents](const StateComponent *observedSC) {
                                    return std::ranges::find(modifiedComponents, observedSC) ==
                                           modifiedComponents.end();
                                }))
                    {
                        nextObserverFound = true;

                        m_SortedObservers.push_back(observer);
                        SwapRemove(observersToSort, observer);
                        for (const StateComponent *modifiedSC: observer->GetModifiedComponents())
                        {
                            SwapRemove(modifiedComponents, modifiedSC);
                        }
                        break;
                    }
                }
            }

            if (!nextObserverFound || !observersToSort.empty())
            {
                throw std::runtime_error("ObserverManager::SortObservers() failed to sort all observers.");
            }
        }

    public:
        /**
         * \brief Adds an observer to the manager.
         * \param observer The observer to add.
         */
        void AddObserver(Observer *observer)
        {
            m_Observers.push_back(observer);
            m_NeedsSorting = true;
        }

        /**
         * \brief Removes an observer from the manager.
         * \param observer The observer to remove.
         */
        void RemoveObserver(Observer *observer)
        {
            std::erase(m_Observers, observer);
            m_DeletedObservers.push_back(observer);
            m_NeedsSorting = true;
        }

        /**
         * \brief Notifies all observers in the correct order.
         *
         * If sorting is required, it sorts the observers first. Deleted observers are skipped.
         */
        void NotifyObservers()
        {
            if (m_NeedsSorting)
            {
                SortObservers();
                m_DeletedObservers.clear();
                m_NeedsSorting = false;
            }

            for (auto observer: m_SortedObservers)
            {
                if (std::ranges::find(m_DeletedObservers, observer) != m_DeletedObservers.end())
                {
                    continue;
                }

                observer->Update();
            }
        }
    };
}
