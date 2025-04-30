#pragma once
#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class SetPresetExpanded : public ZooLib::Command
    {
        bool m_Expanded{};

    public:
        explicit SetPresetExpanded(bool expanded)
            : m_Expanded(expanded)
        {
        }

        static void Execute(const SetPresetExpanded &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.SetExpanded(command.m_Expanded);
        }
    };
}
