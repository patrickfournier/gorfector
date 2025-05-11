#include "ZooLib/StateComponent.hpp"
#include "../TestsSupport/StateComponents.hpp"
#include "gtest/gtest.h"

using namespace TestsSupport;

namespace ZooLib
{
    TEST(ZooLib_StateComponentTests, UsingUpdaterIncrementsVersion)
    {
        auto state = new State();
        auto testStateComponent = new StateComponentA(state);
        EXPECT_EQ(1, testStateComponent->GetVersion());

        {
            StateComponentA::Updater updater(testStateComponent);
        }

        EXPECT_EQ(2, testStateComponent->GetVersion());

        delete testStateComponent;
        delete state;
    }

    TEST(ZooLib_StateComponentTests, AddRemoveSelf)
    {
        auto state = new State();
        EXPECT_EQ(nullptr, state->GetStateComponentByType<TestsSupport::StateComponentA>());

        auto testStateComponent = new StateComponentA(state);
        EXPECT_EQ(testStateComponent, state->GetStateComponentByType<TestsSupport::StateComponentA>());

        delete testStateComponent;
        EXPECT_EQ(nullptr, state->GetStateComponentByType<TestsSupport::StateComponentA>());

        delete state;
    }
}
