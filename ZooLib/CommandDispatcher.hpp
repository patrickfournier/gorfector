#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include "Command.hpp"

namespace ZooLib
{
    /**
     * \class CommandDispatcher
     * \brief Manages the dispatching of commands to their respective handlers.
     *
     * The `CommandDispatcher` class provides a mechanism to register, unregister, and dispatch commands
     * to their associated handler functions. It supports polymorphic command handling and allows for
     * hierarchical dispatching by delegating unhandled commands to a parent dispatcher.
     */
    class CommandDispatcher
    {
        /**
         * \class CommandHandlerFuncBase
         * \brief Abstract base class for command handler functions.
         *
         * This class provides a polymorphic interface for invoking command handler functions.
         * Derived classes must implement the `Invoke` method to handle specific command types.
         */
        class CommandHandlerFuncBase
        {
        public:
            virtual ~CommandHandlerFuncBase() = default;

            /**
             * \brief Invokes the handler function with the given command.
             * \param command The command instance to handle.
             */
            virtual void Invoke(const Command &command) = 0;
        };

        /**
         * \brief A wrapper for the handler function of a specific command type.
         *
         * This class encapsulates a handler function for a specific command type, allowing it
         * to be invoked with the appropriate command instance. It inherits from the base
         * `CommandHandlerFuncBase` to provide a polymorphic interface for invoking handlers.
         *
         * \tparam TCommand The type of the command this handler is associated with.
         */
        template<typename TCommand>
        class CommandHandlerFunc final : public CommandHandlerFuncBase
        {
            std::function<void(const TCommand &)> m_Handler; ///< The handler function for the command.

        public:
            /**
             * \brief Constructs a `CommandHandlerFunc` with the given handler function.
             * \param handler A function to handle the command of type `TCommand`.
             */
            explicit CommandHandlerFunc(std::function<void(const TCommand &)> handler)
                : m_Handler(handler)
            {
            }

            /**
             * \brief Invokes the handler function with the given command.
             * \param command The command instance to pass to the handler.
             */
            void Invoke(const Command &command) override
            {
                m_Handler(static_cast<const TCommand &>(command));
            }
        };

        CommandDispatcher *m_Parent{}; ///< Pointer to the parent dispatcher for hierarchical dispatching.
        std::unordered_map<size_t, CommandHandlerFuncBase *> m_CommandHandlers; ///< Map of command handlers by type.

    public:
        /**
         * \brief Constructs a `CommandDispatcher` with an optional parent dispatcher.
         * \param parent A pointer to the parent dispatcher (default is `nullptr`).
         */
        explicit CommandDispatcher(CommandDispatcher *parent = nullptr)
            : m_Parent(parent)
        {
        }

        /**
         * \brief Dispatches a command to its registered handler.
         * \tparam TCommand The type of the command to dispatch.
         * \param command The command instance to dispatch.
         *
         * If no handler is found for the command type, the command is forwarded to the parent dispatcher
         * (if one exists).
         */
        template<typename TCommand>
        void Dispatch(const TCommand &command) // NOLINT(misc-no-recursion)
        {
            auto key = std::type_index(typeid(TCommand)).hash_code();
            auto handler = m_CommandHandlers.find(key);
            if (handler != m_CommandHandlers.end() && handler->second != nullptr)
            {
                handler->second->Invoke(command);
            }
            else
            {
                if (m_Parent != nullptr && m_Parent != this)
                {
                    m_Parent->Dispatch(command);
                }
            }
        }

        /**
         * \brief Registers a handler function for a specific command type.
         * \tparam TCommand The type of the command to handle.
         * \tparam TStateArgs Variadic template for additional state arguments.
         * \param handler A function to handle the command.
         * \param args Additional state arguments to bind to the handler.
         *
         * The handler function must accept a `const TCommand &` as its first parameter.
         */
        template<typename TCommand, typename... TStateArgs>
        void RegisterHandler(void (*handler)(const TCommand &, TStateArgs *...), TStateArgs *...args)
        {
            static_assert(
                    std::is_base_of_v<Command, TCommand>,
                    "The TCommand type parameter of RegisterHandler<T> must derive from Command");

            auto key = std::type_index(typeid(TCommand)).hash_code();

            auto handlerIt = m_CommandHandlers.find(key);
            if (handlerIt != m_CommandHandlers.end())
            {
                delete handlerIt->second;
            }

            auto func = new CommandHandlerFunc<TCommand>(std::bind(handler, std::placeholders::_1, args...));
            m_CommandHandlers[key] = func;
        }

        /**
         * \brief Unregisters the handler function for a specific command type.
         * \tparam TCommand The type of the command to unregister.
         *
         * If a handler is registered for the command type, it is deleted and removed from the map.
         */
        template<typename TCommand>
        void UnregisterHandler()
        {
            static_assert(
                    std::is_base_of_v<Command, TCommand>,
                    "The TCommand type parameter of RegisterHandler<T> must derive from Command");

            auto key = std::type_index(typeid(TCommand)).hash_code();

            auto handlerIt = m_CommandHandlers.find(key);
            if (handlerIt != m_CommandHandlers.end())
            {
                delete handlerIt->second;
            }

            m_CommandHandlers[key] = nullptr;
        }
    };
}
