#pragma once

#include "ZooLib/Command.hpp"

namespace TestsSupport
{
    class CommandA : public ZooLib::Command
    {
        static int s_CommandHandled;

    public:
        static void Reset()
        {
            s_CommandHandled = 0;
        }

        CommandA()
        {
            Reset();
        }

        static void Execute(const CommandA &command)
        {
            s_CommandHandled++;
        }

        [[nodiscard]] static int GetCommandHandled()
        {
            return s_CommandHandled;
        }
    };
}
