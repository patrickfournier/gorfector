#include "ZooLib/StateComponent.hpp"
#include "gtest/gtest.h"

namespace
{
    class TestComponent : public ZooLib::StateComponent
    {
    public:
        explicit TestComponent(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    TEST(StateComponentTests, UsingUpdaterIncrementsVersion)
    {
        auto state = new ZooLib::State();
        auto testStateComponent = new TestComponent(state);
        EXPECT_EQ(1, testStateComponent->GetVersion());

        {
            ZooLib::StateComponent::Updater updater(testStateComponent);
        }

        EXPECT_EQ(2, testStateComponent->GetVersion());
    }
}
