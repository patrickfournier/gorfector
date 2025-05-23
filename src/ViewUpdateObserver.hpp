#pragma once

#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * \class ViewUpdateObserver
     * \brief A class responsible for observing changes in state components and updating the view accordingly.
     * \tparam TView The type of the view to be updated.
     * \tparam TStates The types of the state components to be observed.
     *
     * The ViewUpdateObserver class is a template class that observes changes in state components; when a change is
     * detected, it updates the view by calling the Update method of the view.
     */
    template<typename TView, typename... TStates>
    class ViewUpdateObserver : public ZooLib::Observer
    {
        TView *m_View{};

    protected:
        /**
         * \brief Updates the view by calling its `Update` method with the observed component versions.
         *
         * This method is called when a change in the observed state components is detected.
         * It ensures that the view is updated to reflect the latest state.
         */
        void UpdateImplementation() override
        {
            m_View->Update(m_ObservedComponentVersions);
        }

    public:
        /**
         * \brief Constructs a `ViewUpdateObserver` instance.
         * \param view Pointer to the view to be updated.
         * \param observedStateComponents Pointers to the state components to be observed.
         *
         * Initializes the observer with the given view and state components to monitor.
         */
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
