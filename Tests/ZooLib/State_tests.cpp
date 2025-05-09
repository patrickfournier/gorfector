#define TEST_FRIENDS friend class ZooLib_StateTests_CanAddTwoStateComponentsOfTheSameType_Test;


#include "../TestsSupport/StateComponents.hpp"
#include "ZooLib/StateComponent.hpp"
#include "gtest/gtest.h"

using namespace TestsSupport;

namespace ZooLib
{
    TEST(ZooLib_StateTests, DeleteStateBeforeComponentsWorks)
    {
        auto state = new State();
        auto testStateComponent = new StateComponentA(state);
        auto testStateComponent2 = new StateComponentA(state);

        delete state;
        // The components should be deleted in the destructor of State.
    }

    TEST(ZooLib_StateTests, IsAddedOnlyOnce)
    {
        auto state = new State();
        auto testStateComponent1 = new StateComponentA(state);

        // Add the same component twice
        state->AddStateComponent(testStateComponent1);

        // Remove the component once.
        state->RemoveStateComponent(testStateComponent1);

        // The component should not be present anymore.
        EXPECT_EQ(nullptr, state->GetStateComponentByType<StateComponentA>());

        delete testStateComponent1;
        delete state;
    }

    TEST(ZooLib_StateTests, CanAddTwoStateComponentsOfTheSameType)
    {
        auto state = new State();
        auto testStateComponent1 = new StateComponentA(state);
        auto testStateComponent2 = new StateComponentA(state);

        // Add two components of the same type
        state->AddStateComponent(testStateComponent1);
        state->AddStateComponent(testStateComponent2);

        // Verify both components are present
        EXPECT_EQ(2, state->m_StateComponents.size());

        delete testStateComponent1;
        delete testStateComponent2;
        delete state;
    }
}
