#pragma once

#include "DeviceOptionsState.hpp"
#include "PreviewState.hpp"
#include "ZooLib/Observer.hpp"

namespace Gorfector
{
    /**
     * \class DeviceOptionsObserver
     * \brief Observes changes in DeviceOptionsState and updates PreviewState accordingly.
     *
     * This class inherits from ZooLib::Observer and is responsible for monitoring
     * a `DeviceOptionsState` object and applying updates to a `PreviewState` object
     * when changes occur. It ensures that the scan area in the preview state is
     * synchronized with the device options state.
     */
    class DeviceOptionsObserver final : public ZooLib::Observer
    {
    protected:
        /**
         * \brief Updates the modified state component based on the observed state component.
         *
         * This method is called when the observed state changes. It retrieves the
         * `DeviceOptionsState` and `PreviewState` components, and updates the scan
         * area in the preview state using the data from the device options state.
         */
        void UpdateImplementation() override
        {
            const auto deviceOptionsState = dynamic_cast<const DeviceOptionsState *>(m_ObservedComponents[0]);
            auto appState = dynamic_cast<PreviewState *>(m_ModifiedComponents[0]);
            auto updater = PreviewState::Updater(appState);
            updater.UpdateScanArea(deviceOptionsState->GetScanArea());
        }

    public:
        /**
         * \brief Constructs a DeviceOptionsObserver.
         *
         * \param observedStateComponent Pointer to the `DeviceOptionsState` to be observed.
         * \param modifiedStateComponent Pointer to the `PreviewState` to be updated.
         */
        DeviceOptionsObserver(const DeviceOptionsState *observedStateComponent, PreviewState *modifiedStateComponent)
            : Observer(
                      std::vector<const ZooLib::StateComponent *>({observedStateComponent}),
                      std::vector<ZooLib::StateComponent *>({modifiedStateComponent}))
        {
        }
    };
}
