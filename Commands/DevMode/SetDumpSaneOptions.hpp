#pragma once
#include "AppState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    class SetDumpSaneOptions : public ZooLib::Command
    {
        bool m_DumpSane;

    public:
        explicit SetDumpSaneOptions(bool dumpSane)
            : m_DumpSane(dumpSane)
        {
        }

        static void Execute(const SetDumpSaneOptions &command, DeviceSelectorState *state)
        {
            auto updater = DeviceSelectorState::Updater(state);
            updater.SetDumpSaneOptions(command.m_DumpSane);
        }
    };
}
