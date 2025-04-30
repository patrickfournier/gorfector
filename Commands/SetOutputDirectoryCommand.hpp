#pragma once
#include <filesystem>

#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class SetOutputDirectoryCommand : public ZooLib::Command
    {
        std::filesystem::path m_OutputDirectory;

    public:
        explicit SetOutputDirectoryCommand(const std::filesystem::path &outputDirectory)
            : m_OutputDirectory(outputDirectory)
        {
        }

        static void Execute(const SetOutputDirectoryCommand &command, Gorfector::OutputOptionsState *outputOptionsState)
        {
            auto updater = Gorfector::OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputDirectory(command.m_OutputDirectory);
        }
    };
}
