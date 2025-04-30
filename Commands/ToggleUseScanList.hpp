#pragma once

#include "AppState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    struct ToggleUseScanList : public ZooLib::Command
    {
        ToggleUseScanList() = default;

        static void Execute(const ToggleUseScanList &command, AppState *appState)
        {
            auto updater = AppState::Updater(appState);
            updater.SetUseScanList(!appState->GetUseScanList());
        }
    };
}
