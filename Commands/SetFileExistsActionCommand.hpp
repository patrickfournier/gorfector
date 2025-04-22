#pragma once
#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetFileExistsActionCommand : public ZooLib::Command
    {
        OutputOptionsState::FileExistsAction m_Action;

    public:
        explicit SetFileExistsActionCommand(OutputOptionsState::FileExistsAction action)
            : m_Action(action)
        {
        }

        static void Execute(const SetFileExistsActionCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetFileExistsAction(command.m_Action);
        }
    };

}
