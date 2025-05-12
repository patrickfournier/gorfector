#pragma once
#include "Command.hpp"
#include "ScanListState.hpp"

namespace Gorfector
{
    /**
     * \class SetAddToScanListAddsAllParamsCommand
     * \brief Command to set whether the "Add to Scan List" button adds all parameters or just the scan area.
     */
    class SetAddToScanListAddsAllParamsCommand : public ZooLib::Command
    {
        /**
         * \brief Indicates whether the "Add to Scan List" button should add all parameters.
         */
        bool m_AddsAllParams;

    public:
        /**
         * \brief Constructor for the command.
         * \param addsAllParams Boolean indicating the desired behavior of the "Add to Scan List" button.
         */
        explicit SetAddToScanListAddsAllParamsCommand(bool addsAllParams)
            : m_AddsAllParams(addsAllParams)
        {
        }

        /**
         * \brief Executes the command to update the state of the "Add to Scan List" button.
         * \param command The command instance containing the desired state.
         * \param state Pointer to the `ScanListState` to be updated.
         */
        static void Execute(const SetAddToScanListAddsAllParamsCommand &command, ScanListState *state)
        {
            auto updater = ScanListState::Updater(state);
            updater.SetAddToScanListAddsAllParams(command.m_AddsAllParams);
        }
    };
}
