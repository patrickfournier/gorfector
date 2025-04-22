#pragma once
#include <string>


#include "OutputOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class SetOutputFileNameCommand : public ZooLib::Command
    {
        std::string m_OutputFileName;

    public:
        explicit SetOutputFileNameCommand(const std::string &outputFileName)
            : m_OutputFileName(outputFileName)
        {
        }

        static void Execute(const SetOutputFileNameCommand &command, OutputOptionsState *outputOptionsState)
        {
            auto updater = OutputOptionsState::Updater(outputOptionsState);
            updater.SetOutputFileName(command.m_OutputFileName);
        }
    };

}
