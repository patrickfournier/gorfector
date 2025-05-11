#pragma once

#include <string>

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetOutputFileNameCommand
     * \brief Command class to set the output file name in the `OutputOptionsState`.
     *
     * This class encapsulates the logic for updating the output file name
     * in the `OutputOptionsState`.
     */
    class SetOutputFileNameCommand : public ZooLib::Command
    {
        /**
         * \brief The desired output file name.
         */
        std::string m_OutputFileName;

    public:
        /**
         * \brief Constructor for the SetOutputFileNameCommand.
         * \param outputFileName The desired output file name to set.
         */
        explicit SetOutputFileNameCommand(const std::string &outputFileName)
            : m_OutputFileName(outputFileName)
        {
        }

        /**
         * \brief Executes the command to set the output file name.
         * \param command The `SetOutputFileNameCommand` instance containing the desired file name.
         * \param outputOptionsState Pointer to the `OutputOptionsState` where the file name will be updated.
         */
        static void Execute(const SetOutputFileNameCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputFileName(command.m_OutputFileName);
        }
    };
}
