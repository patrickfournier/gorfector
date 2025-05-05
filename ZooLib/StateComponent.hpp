#pragma once

#include "State.hpp"

namespace ZooLib
{
    /**
     * \class StateComponent
     * \brief Represents a part of the application state.
     *
     * This class provides a base for components that are part of a State.
     * It manages a reference to the State and tracks a version number for the component.
     *
     * Derived classes must make sure that they only provide read-only access to the `StateComponent`.
     * All modifications should be done through the Updater class.
     */
    class StateComponent
    {
    protected:
        /**
         * \brief Pointer to the associated State object.
         */
        State *m_State{};

    private:
        /**
         * \brief Tracks the version of the StateComponent.
         */
        uint64_t m_StateVersion{};

    public:
        /**
         * \brief Deleted default constructor to enforce initialization with a State.
         */
        StateComponent() = delete;

        /**
         * \brief Constructs a StateComponent and registers it with the given State.
         * \param state Pointer to the State object this component belongs to.
         */
        explicit StateComponent(State *state)
            : m_State(state)
            , m_StateVersion(1)
        {
            m_State->AddStateComponent(this);
        }

        /**
         * \brief Destructor that unregisters the component from the State.
         */
        virtual ~StateComponent()
        {
            m_State->RemoveStateComponent(this);
        };

        /**
         * \brief Retrieves the current version of the StateComponent.
         * \return The version number.
         */
        [[nodiscard]] uint64_t GetVersion() const
        {
            return m_StateVersion;
        }

        /**
         * \brief Retrieves the serialization key for the component.
         * \return A string representing the serialization key. Default is an empty string, which means no
         * serialization.
         */
        [[nodiscard]] virtual std::string GetSerializationKey() const
        {
            return {};
        }

        /**
         * \class Updater
         * \brief A helper class to manage updates to a StateComponent.
         *
         * This templated class ensures that updates to a StateComponent are properly
         * handled and the version is incremented after updates.
         * \tparam TState The type of the StateComponent being updated.
         */
        template<typename TState>
        class Updater
        {
        protected:
            /**
             * \brief Pointer to the StateComponent being updated.
             */
            TState *m_StateComponent{};

        public:
            /**
             * \brief Constructs an Updater for the given StateComponent.
             * \param stateComponent Pointer to the StateComponent to be updated.
             */
            explicit Updater(TState *stateComponent)
                : m_StateComponent(stateComponent)
            {
                static_assert(
                        std::is_base_of_v<StateComponent, TState>,
                        "The type parameter of Updater<T> must derive from StateComponent");
            }

            /**
             * \brief Destructor that increments the version of the StateComponent.
             */
            virtual ~Updater()
            {
                ++m_StateComponent->m_StateVersion;
            }

            /**
             * \brief Loads data into the StateComponent from a JSON object.
             * \param json The JSON object containing the data.
             */
            virtual void LoadFromJson(const nlohmann::json &json) = 0;
        };
    };

}
