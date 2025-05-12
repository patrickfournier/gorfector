#pragma once
#include "Command.hpp"
#include "ScanListState.hpp"

namespace Gorfector
{
    /**
     * \class ClearScanListCommand
     * \brief Command to clear the current scan list in the `ScanListState`.
     *
     * This class provides functionality to execute a command that clears all scan items
     * from the current scan list in the `ScanListState`. It uses the `Updater` class
     * from `ScanListState` to perform the operation.
     */
    class ClearScanListCommand : public ZooLib::Command
    {
    public:
        /**
         * \brief Default constructor for the ClearScanListCommand.
         */
        ClearScanListCommand() = default;

        /**
         * \brief Executes the command to clear the scan list.
         * \param command The command instance (not used in this implementation).
         * \param state Pointer to the `ScanListState` to be updated.
         */
        static void Execute(const ClearScanListCommand &command, ScanListState *state)
        {
            auto updater = ScanListState::Updater(state);
            updater.ClearScanList();
        }
    };
}
