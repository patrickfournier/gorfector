#pragma once
#include "AppState.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/Observer.hpp"

namespace ZooScan
{

    /**
     * @class CurrentDeviceObserver
     * @brief A class responsible for observing the current device and updating the preset panel state.
     */
    class CurrentDeviceObserver : public ZooLib::Observer
    {
        const AppState *m_AppState;
        const DeviceSelectorState *m_DeviceSelectorState;
        PresetPanelState *m_PresetPanelState;

    protected:
        void UpdateImplementation() override
        {
            auto currentDeviceName = m_AppState->GetCurrentDeviceName();
            auto device = m_DeviceSelectorState->GetDeviceByName(currentDeviceName);
            std::string vendorName = device == nullptr ? "" : device->GetVendor();
            std::string modelName = device == nullptr ? "" : device->GetModel();
            auto updater = PresetPanelState::Updater(m_PresetPanelState);
            updater.SetCurrentDeviceName(vendorName, modelName);
        }

    public:
        CurrentDeviceObserver(
                const AppState *appState, const DeviceSelectorState *deviceSelectorState,
                PresetPanelState *presetPanelState)
            : Observer({appState, deviceSelectorState}, {presetPanelState})
            , m_AppState(appState)
            , m_DeviceSelectorState(deviceSelectorState)
            , m_PresetPanelState(presetPanelState)
        {
        }
    };

}
