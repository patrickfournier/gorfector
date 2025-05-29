#pragma once

#include "AppState.hpp"
#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * \brief A template class that observes changes in the current device state.
     *
     * This class listens for changes in the application state and updates the modified state
     * with the current device's vendor and model information when a relevant change is detected.
     *
     * \tparam TModifiedState The type of the modified state that will be updated.
     */
    template<typename TModifiedState>
    class CurrentDeviceObserver : public ZooLib::Observer
    {
        /**
         * \brief Pointer to the application state being observed.
         */
        const AppState *m_AppState;

        /**
         * \brief Pointer to the device selector state used to retrieve device information.
         */
        const DeviceSelectorState *m_DeviceSelectorState;

        /**
         * \brief Pointer to the modified state that will be updated.
         */
        TModifiedState *m_ModifiedState;

    protected:
        /**
         * \brief Updates the modified state when the current device changes.
         *
         * This method checks for changes in the current device and updates the modified state
         * with the vendor and model information of the current device.
         */
        void UpdateImplementation() override
        {
            auto changeset = m_AppState->GetAggregatedChangeset(m_ObservedComponentVersions[0]);
            if (changeset != nullptr && changeset->IsChanged(AppStateChangeset::ChangeTypeFlag::e_CurrentDevice))
            {
                auto currentDeviceName = m_AppState->GetCurrentDeviceName();
                auto device = m_DeviceSelectorState->GetDeviceByName(currentDeviceName);
                std::string vendorName = device == nullptr ? "" : device->GetVendor();
                std::string modelName = device == nullptr ? "" : device->GetModel();
                auto updater = typename TModifiedState::Updater(m_ModifiedState);
                updater.SetCurrentDeviceName(vendorName, modelName);
            }
            if (changeset != nullptr && changeset->IsChanged(AppStateChangeset::ChangeTypeFlag::e_ScanActivity))
            {
                auto scanActivity = m_AppState->IsPreviewing() || m_AppState->IsScanning();
                auto updater = typename TModifiedState::Updater(m_ModifiedState);
                updater.SetScanActivity(scanActivity);
            }
        }

    public:
        /**
         * \brief Constructs a CurrentDeviceObserver.
         *
         * \param appState Pointer to the application state to observe.
         * \param deviceSelectorState Pointer to the device selector state for retrieving device information.
         * \param modifiedState Pointer to the modified state to be updated.
         */
        CurrentDeviceObserver(
                const AppState *appState, const DeviceSelectorState *deviceSelectorState, TModifiedState *modifiedState)
            : Observer({appState, deviceSelectorState}, {modifiedState})
            , m_AppState(appState)
            , m_DeviceSelectorState(deviceSelectorState)
            , m_ModifiedState(modifiedState)
        {
        }
    };
}
