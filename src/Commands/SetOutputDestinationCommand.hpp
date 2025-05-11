#pragma once

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetOutputDestinationCommand
     * \brief Command class to set the output destination in the `OutputOptionsState`.
     *
     * This class encapsulates the logic for updating the output destination
     * in the `OutputOptionsState`.
     */
    class SetOutputDestinationCommand : public ZooLib::Command
    {
        /**
         * \brief The desired output destination.
         */
        OutputOptionsState::OutputDestination m_OutputDestination;

    public:
        /**
         * \brief Constructor for the SetOutputDestinationCommand.
         * \param outputDestination The desired output destination to set.
         */
        explicit SetOutputDestinationCommand(OutputOptionsState::OutputDestination outputDestination)
            : m_OutputDestination(outputDestination)
        {
        }

        /**
         * \brief Executes the command to set the output destination.
         * \param command The `SetOutputDestinationCommand` instance containing the desired destination.
         * \param outputOptionsState Pointer to the `OutputOptionsState` where the destination will be updated.
         */
        static void Execute(const SetOutputDestinationCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputDestination(command.m_OutputDestination);
        }
    };
}
