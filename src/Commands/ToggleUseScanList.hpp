#pragma once

#include "AppState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class ToggleUseScanList
     * \brief Command class to toggle the use of the scan list in the `AppState`.
     *
     * This class encapsulates the logic for toggling the `UseScanList` setting
     * in the `AppState`.
     */
    class ToggleUseScanList : public ZooLib::Command
    {
    public:
        /**
         * \brief Executes the command to toggle the `UseScanList` setting.
         * \param command The `ToggleUseScanList` instance (not used in this implementation).
         * \param appState Pointer to the `AppState` where the `UseScanList` setting will be toggled.
         */
        static void Execute(const ToggleUseScanList &command, AppState *appState)
        {
            auto updater = AppState::Updater(appState);
            updater.SetUseScanList(!appState->GetUseScanList());
        }
    };
}
