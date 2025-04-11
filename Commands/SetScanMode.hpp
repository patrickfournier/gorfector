#pragma once

#include "AppState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    struct SetSingleScanMode : public ZooLib::Command
    {

    public:
        SetSingleScanMode()
        {
        }

        static void Execute(const SetSingleScanMode &command, AppState *appState)
        {
            auto updater = AppState::Updater(appState);
            updater.SetScanMode(AppState::Single);
        }
    };

    struct SetBatchScanMode : public ZooLib::Command
    {

    public:
        SetBatchScanMode()
        {
        }

        static void Execute(const SetBatchScanMode &command, AppState *appState)
        {
            auto updater = AppState::Updater(appState);
            updater.SetScanMode(AppState::Batch);
        }
    };
}
