#pragma once

#include <vector>
#include <algorithm>
#include "Observer.h"

namespace Zoo
{
    class ObserverManager
    {
        std::vector<Zoo::Observer *> m_Observers;

    public:
        void AddObserver(Zoo::Observer *observer)
        {
            m_Observers.push_back(observer);
        }

        void RemoveObserver(Zoo::Observer *observer)
        {
            m_Observers.erase(std::remove(m_Observers.begin(), m_Observers.end(), observer), m_Observers.end());
        }

        void NotifyObservers()
        {
            for (auto observer : m_Observers)
            {
                observer->Update();
            }
        }
    };
}
