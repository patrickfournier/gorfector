#pragma once
#include "ZooLib/StateComponent.hpp"

namespace TestsSupport
{
    class StateComponentA : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentA(ZooLib::State *state)
            : StateComponent{state}
        {
        }

        class Updater : public StateComponent::Updater<StateComponentA>
        {
        public:
            explicit Updater(StateComponentA *state)
                : StateComponent::Updater<StateComponentA>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
                // No implementation needed for this test
            }
        };
    };

    class StateComponentB : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentB(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    class StateComponentC : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentC(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    class StateComponentD : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentD(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };
}
