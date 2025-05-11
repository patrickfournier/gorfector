#pragma once
#include <string>

#include "ZooLib/StateComponent.hpp"

namespace TestsSupport
{
    class Log : public ZooLib::StateComponent
    {
        std::string m_Log{};

    public:
        Log() = delete;

        explicit Log(ZooLib::State *state)
            : StateComponent{state}
        {
        }

        void Push(char c)
        {
            m_Log.append(1, c);
        }

        const std::string &GetLog() const
        {
            return m_Log;
        }
    };
}
