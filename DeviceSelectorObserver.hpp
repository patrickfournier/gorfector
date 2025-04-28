#pragma once

#include "AppState.hpp"
#include "DeviceSelectorState.hpp"

#include "ZooLib/Observer.hpp"


namespace ZooScan
{
    /**
     * @class DeviceSelectorObserver
     * @brief A class responsible for observing which device is selected and responding to updates.
     *
     * The DeviceSelectorObserver observing which device is selected and notifies the AppState
     * about the change.
     */
    class DeviceSelectorObserver final : public ZooLib::Observer
    {
    protected:
        void UpdateImplementation() override
        {
            const auto deviceSelectorState = dynamic_cast<const DeviceSelectorState *>(m_ObservedComponents[0]);
            auto appState = dynamic_cast<AppState *>(m_ModifiedComponents[0]);
            auto updater = AppState::Updater(appState);
            updater.SetCurrentDevice(deviceSelectorState->GetSelectedDeviceName());
        }

    public:
        DeviceSelectorObserver(const DeviceSelectorState *observedStateComponent, AppState *modifiedStateComponent)
            : Observer(
                      std::vector<const ZooLib::StateComponent *>({observedStateComponent}),
                      std::vector<ZooLib::StateComponent *>({modifiedStateComponent}))
        {
        }
    };
}
