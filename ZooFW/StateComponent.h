#pragma once

#include <cstdlib>
#include <random>

namespace Zoo
{
    class StateComponent
    {
        uint64_t m_StateVersion{};

    public:
        StateComponent()
        {
            m_StateVersion = 1;
        }

        virtual ~StateComponent() = default;

        [[nodiscard]] uint64_t Version() const
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

            virtual ~Updater()
            {
                m_StateComponent->m_StateVersion++;
            }
        };
    };

}
