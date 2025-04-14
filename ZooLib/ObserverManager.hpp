#pragma once

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "Observer.hpp"

namespace ZooLib
{
    class ObserverManager
    {
        std::vector<Observer *> m_Observers{};

        bool m_NeedsSorting{};
        std::vector<Observer *> m_SortedObservers{};
        std::vector<Observer *> m_DeletedObservers{};

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

        void SortObservers()
        {
            // Observers establish a dependency between their observed and modified state components,
            // thus forming a directed graph (hopefully acyclic).
            // Do a topological sort on the graph to execute the observers in the correct order.
            // See https://en.wikipedia.org/wiki/Topological_sorting.
            m_SortedObservers.clear();
            std::vector observersToSort(m_Observers);
            std::vector<const StateComponent *> modifiedComponents;
            for (const Observer *observer: observersToSort)
            {
                for (auto modifiedSC: observer->ModifiedComponents())
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
                                observer->ObservedComponents(),
                                [&modifiedComponents](const StateComponent *observedSC) {
                                    return std::ranges::find(modifiedComponents, observedSC) ==
                                           modifiedComponents.end();
                                }))
                    {
                        nextObserverFound = true;

                        m_SortedObservers.push_back(observer);
                        SwapRemove(observersToSort, observer);
                        for (const StateComponent *modifiedSC: observer->ModifiedComponents())
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
        void AddObserver(Observer *observer)
        {
            m_Observers.push_back(observer);
            m_NeedsSorting = true;
        }

        void RemoveObserver(Observer *observer)
        {
            std::erase(m_Observers, observer);
            m_DeletedObservers.push_back(observer);
            m_NeedsSorting = true;
        }

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
