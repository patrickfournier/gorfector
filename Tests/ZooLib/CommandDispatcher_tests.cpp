#include "ZooLib/CommandDispatcher.hpp"

#include "../TestsSupport/Commands.hpp"
#include "gtest/gtest.h"
namespace ZooLib
{
    TEST(ZooLib_CommandDispatcherTests, RegisteredCommandCanBeDispatched)
    {
        CommandDispatcher dispatcher;

        dispatcher.RegisterHandler<TestsSupport::CommandA>(TestsSupport::CommandA::Execute);

        TestsSupport::CommandA command;
        dispatcher.Dispatch(command);

        EXPECT_EQ(1, command.GetCommandHandled());
    }

    TEST(ZooLib_CommandDispatcherTests, UnregisteredCommandCannotBeDispatched)
    {
        CommandDispatcher dispatcher;

        dispatcher.RegisterHandler<TestsSupport::CommandA>(TestsSupport::CommandA::Execute);
        dispatcher.UnregisterHandler<TestsSupport::CommandA>();

        TestsSupport::CommandA command;
        dispatcher.Dispatch(command);

        EXPECT_EQ(0, command.GetCommandHandled());
    }
}
