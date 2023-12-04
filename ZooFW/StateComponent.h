#pragma once

#include <cstdlib>
#include <random>

namespace Zoo
{
    class StateComponent
    {
        u_int32_t m_StateId{};
        u_int64_t m_StateVersion{};

    public:
        StateComponent()
        {
            // get a random number for the state id
            m_StateId = std::random_device()();
            m_StateVersion = 1;
        }

        [[nodiscard]] u_int64_t Version() const
        {
            return m_StateVersion;
        }

        template<typename TState>
        class Updater
        {
        protected:
            TState *m_StateComponent{};

        public:
            explicit Updater(TState *stateComponent)
                    : m_StateComponent(stateComponent)
            {
                static_assert(std::is_base_of<StateComponent, TState>::value,
                              "The type parameter of Updater<T> must derive from StateComponent");
            }

            ~Updater()
            {
                m_StateComponent->m_StateVersion++;
            }
        };
    };

}
