#pragma once

#include "Command.h"
#include <typeindex>
#include <unordered_map>
#include <functional>

namespace Zoo
{
    class CommandDispatcher
    {
        class CommandHandlerFuncBase
        {
        public:
            virtual ~CommandHandlerFuncBase() = default;

            virtual void Invoke(const Command &command) = 0;
        };

        template<typename TCommand>
        class CommandHandlerFunc : public CommandHandlerFuncBase
        {
            std::function<void(const TCommand &)> m_Handler;

        public:
            explicit CommandHandlerFunc(std::function<void(const TCommand &)> handler)
                    : m_Handler(handler)
            {}

            void Invoke(const Command &command) override
            {
                m_Handler(static_cast<const TCommand &>(command));
            }
        };

        CommandDispatcher *m_Parent{};
        std::unordered_map<size_t, CommandHandlerFuncBase *> m_CommandHandlers;

    public:
        explicit CommandDispatcher(CommandDispatcher *parent = nullptr)
                : m_Parent(parent)
        {}

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

        template<typename TCommand, typename ...TStateArgs>
        void RegisterHandler(void (*handler)(const TCommand &, TStateArgs *...), TStateArgs* ...args)
        {
            static_assert(std::is_base_of<Command, TCommand>::value,
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

        template<typename TCommand>
        void UnregisterHandler()
        {
            static_assert(std::is_base_of<Command, TCommand>::value,
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
