#pragma once

#include <filesystem>

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetOutputDirectoryCommand
     * \brief Command class to set the output directory in the `OutputOptionsState`.
     *
     * This class encapsulates the logic for updating the output directory
     * in the `OutputOptionsState`.
     */
    class SetOutputDirectoryCommand : public ZooLib::Command
    {
        /**
         * \brief The desired output directory.
         */
        std::filesystem::path m_OutputDirectory;

    public:
        /**
         * \brief Constructor for the SetOutputDirectoryCommand.
         * \param outputDirectory The desired output directory to set.
         */
        explicit SetOutputDirectoryCommand(const std::filesystem::path &outputDirectory)
            : m_OutputDirectory(outputDirectory)
        {
        }

        /**
         * \brief Executes the command to set the output directory.
         * \param command The `SetOutputDirectoryCommand` instance containing the desired directory.
         * \param outputOptionsState Pointer to the `OutputOptionsState` where the directory will be updated.
         */
        static void Execute(const SetOutputDirectoryCommand &command, Gorfector::OutputOptionsState *outputOptionsState)
        {
            auto updater = Gorfector::OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputDirectory(command.m_OutputDirectory);
        }
    };
}
