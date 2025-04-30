#pragma once
#include "AppState.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * @class CurrentDeviceObserver
     * @brief A class responsible for observing the current device and updating the preset panel state.
     */
    template<typename TModifiedState>
    class CurrentDeviceObserver : public ZooLib::Observer
    {
        const AppState *m_AppState;
        const DeviceSelectorState *m_DeviceSelectorState;
        TModifiedState *m_ModifiedState;

    protected:
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
        }

    public:
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
