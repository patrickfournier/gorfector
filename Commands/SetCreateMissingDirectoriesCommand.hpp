#pragma once
#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetCreateMissingDirectoriesCommand : public ZooLib::Command
    {
        bool m_CreateMissingDirectories{};

    public:
        explicit SetCreateMissingDirectoriesCommand(bool createMissingDirectories)
            : m_CreateMissingDirectories(createMissingDirectories)
        {
        }

        static void Execute(const SetCreateMissingDirectoriesCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetCreateMissingDirectories(command.m_CreateMissingDirectories);
        }
    };

}
