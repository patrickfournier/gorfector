#pragma once

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetCreateMissingDirectoriesCommand
     * \brief Command class to set the option for creating missing directories in the `OutputOptionsState`.
     *
     * This class encapsulates the logic for enabling or disabling the creation of missing directories
     * in the `OutputOptionsState`.
     */
    class SetCreateMissingDirectoriesCommand : public ZooLib::Command
    {
        /**
         * \brief Indicates whether missing directories should be created.
         */
        bool m_CreateMissingDirectories{};

    public:
        /**
         * \brief Constructor for the SetCreateMissingDirectoriesCommand.
         * \param createMissingDirectories A boolean indicating whether to enable or disable the creation of missing
         * directories.
         */
        explicit SetCreateMissingDirectoriesCommand(bool createMissingDirectories)
            : m_CreateMissingDirectories(createMissingDirectories)
        {
        }

        /**
         * \brief Executes the command to set the option for creating missing directories.
         * \param command The `SetCreateMissingDirectoriesCommand` instance containing the desired option.
         * \param outputOptionsState Pointer to the `OutputOptionsState` where the option will be updated.
         */
        static void Execute(const SetCreateMissingDirectoriesCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetCreateMissingDirectories(command.m_CreateMissingDirectories);
        }
    };
}
