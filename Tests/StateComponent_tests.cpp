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

        class Updater : public StateComponent::Updater<TestComponent>
        {
        public:
            explicit Updater(TestComponent *state)
                : StateComponent::Updater<TestComponent>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                // No implementation needed for this test
            }
        };
    };

    TEST(StateComponentTests, UsingUpdaterIncrementsVersion)
    {
        auto state = new ZooLib::State();
        auto testStateComponent = new TestComponent(state);
        EXPECT_EQ(1, testStateComponent->GetVersion());

        {
            TestComponent::Updater updater(testStateComponent);
        }

        EXPECT_EQ(2, testStateComponent->GetVersion());
    }
}
