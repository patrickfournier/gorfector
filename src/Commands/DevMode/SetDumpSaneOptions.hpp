#pragma once

#include "AppState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetDumpSaneOptions
     * \brief Command class to set the `DumpSaneOptions` flag in the `DeviceSelectorState`.
     *
     * This class encapsulates the logic for updating the `DumpSaneOptions` setting
     * in the `DeviceSelectorState`.
     */
    class SetDumpSaneOptions : public ZooLib::Command
    {
        /**
         * \brief Flag indicating whether to enable or disable `DumpSaneOptions`.
         */
        bool m_DumpSane;

    public:
        /**
         * \brief Constructor for the `SetDumpSaneOptions` command.
         * \param dumpSane The desired state of the `DumpSaneOptions` flag.
         */
        explicit SetDumpSaneOptions(bool dumpSane)
            : m_DumpSane(dumpSane)
        {
        }

        /**
         * \brief Executes the command to update the `DumpSaneOptions` setting.
         * \param command The `SetDumpSaneOptions` instance containing the desired setting.
         * \param state Pointer to the `DeviceSelectorState` where the setting will be updated.
         */
        static void Execute(const SetDumpSaneOptions &command, DeviceSelectorState *state)
        {
            auto updater = DeviceSelectorState::Updater(state);
            updater.SetDumpSaneOptions(command.m_DumpSane);
        }
    };
}
