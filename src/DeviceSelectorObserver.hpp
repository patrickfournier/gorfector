#pragma once

#include "AppState.hpp"
#include "DeviceSelectorState.hpp"
#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * \class DeviceSelectorObserver
     * \brief Observes the selected device and updates the application state accordingly.
     *
     * The `DeviceSelectorObserver` monitors changes in the `DeviceSelectorState` and updates
     * the `AppState` to reflect the currently selected device. It ensures synchronization
     * between the device selection UI and the application state.
     */
    class DeviceSelectorObserver final : public ZooLib::Observer
    {
    protected:
        /**
         * \brief Updates the application state based on the observed device selector state.
         *
         * This method is called when the observed state changes. It retrieves the selected
         * device name from the `DeviceSelectorState` and updates the `AppState` with the
         * new selection.
         */
        void UpdateImplementation() override
        {
            const auto deviceSelectorState = dynamic_cast<const DeviceSelectorState *>(m_ObservedComponents[0]);
            auto appState = dynamic_cast<AppState *>(m_ModifiedComponents[0]);
            auto updater = AppState::Updater(appState);
            updater.SetCurrentDevice(deviceSelectorState->GetSelectedDeviceName());
        }

    public:
        /**
         * \brief Constructs a `DeviceSelectorObserver` instance.
         * \param observedStateComponent Pointer to the `DeviceSelectorState` being observed.
         * \param modifiedStateComponent Pointer to the `AppState` to be updated.
         *
         * Initializes the observer with the components it observes and modifies.
         */
        DeviceSelectorObserver(const DeviceSelectorState *observedStateComponent, AppState *modifiedStateComponent)
            : Observer(
                      std::vector<const ZooLib::StateComponent *>({observedStateComponent}),
                      std::vector<ZooLib::StateComponent *>({modifiedStateComponent}))
        {
        }
    };
}
