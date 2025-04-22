#pragma once
#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetOutputDestinationCommand : public ZooLib::Command
    {
        OutputOptionsState::OutputDestination m_OutputDestination;

    public:
        explicit SetOutputDestinationCommand(OutputOptionsState::OutputDestination outputDestination)
            : m_OutputDestination(outputDestination)
        {
        }

        static void Execute(const SetOutputDestinationCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputDestination(command.m_OutputDestination);
        }
    };

}
