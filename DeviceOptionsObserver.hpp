#pragma once

#include "DeviceOptionsState.hpp"
#include "PreviewState.hpp"
#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * @class DeviceOptionsObserver
     * @brief A class responsible for observing changes in device options and responding to updates.
     *
     * The DeviceOptionsObserver class monitors the device options and updates the preview whenever the
     * scan area is modified.
     */
    class DeviceOptionsObserver final : public ZooLib::Observer
    {
    protected:
        void UpdateImplementation() override
        {
            const auto deviceOptionsState = dynamic_cast<const DeviceOptionsState *>(m_ObservedComponents[0]);
            auto appState = dynamic_cast<PreviewState *>(m_ModifiedComponents[0]);
            auto updater = PreviewState::Updater(appState);
            updater.UpdateScanArea(deviceOptionsState->GetScanArea());
        }

    public:
        DeviceOptionsObserver(const DeviceOptionsState *observedStateComponent, PreviewState *modifiedStateComponent)
            : Observer(
                      std::vector<const ZooLib::StateComponent *>({observedStateComponent}),
                      std::vector<ZooLib::StateComponent *>({modifiedStateComponent}))
        {
        }
    };
}
