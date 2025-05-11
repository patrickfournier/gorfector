#pragma once

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetFileExistsActionCommand
     * \brief Command class to set the action to take when a file already exists in the `OutputOptionsState`.
     *
     * This class encapsulates the logic for updating the file existence action in the `OutputOptionsState`.
     */
    class SetFileExistsActionCommand : public ZooLib::Command
    {
        /**
         * \brief The action to take when a file already exists.
         */
        OutputOptionsState::FileExistsAction m_Action;

    public:
        /**
         * \brief Constructor for the SetFileExistsActionCommand.
         * \param action The action to take when a file already exists.
         */
        explicit SetFileExistsActionCommand(OutputOptionsState::FileExistsAction action)
            : m_Action(action)
        {
        }

        /**
         * \brief Executes the command to set the file existence action.
         * \param command The `SetFileExistsActionCommand` instance containing the desired action.
         * \param outputOptionsState Pointer to the `OutputOptionsState` where the action will be updated.
         */
        static void Execute(const SetFileExistsActionCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetFileExistsAction(command.m_Action);
        }
    };
}
