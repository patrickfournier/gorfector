#pragma once

#include "State.hpp"

namespace ZooLib
{
    class StateComponent
    {
    protected:
        State *m_State{};

    private:
        uint64_t m_StateVersion{};

    public:
        StateComponent() = delete;

        explicit StateComponent(State *state)
            : m_State(state)
            , m_StateVersion(1)
        {
            m_State->AddStateComponent(this);
        }

        virtual ~StateComponent()
        {
            m_State->RemoveStateComponent(this);
        };

        [[nodiscard]] uint64_t GetVersion() const
        {
            return m_StateVersion;
        }

        [[nodiscard]] virtual std::string GetSerializationKey() const
        {
            return {};
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
                static_assert(
                        std::is_base_of_v<StateComponent, TState>,
                        "The type parameter of Updater<T> must derive from StateComponent");
            }

            virtual ~Updater()
            {
                ++m_StateComponent->m_StateVersion;
            }

            virtual void LoadFromJson(const nlohmann::json &json) = 0;
        };
    };

}
