#pragma once
#include <string>


#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{
    class DeletePresetCommand : public ZooLib::Command
    {
        std::string m_PresetId{};

    public:
        explicit DeletePresetCommand(std::string presetId)
            : m_PresetId(presetId)
        {
        }

        static void Execute(const DeletePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.RemovePreset(command.m_PresetId);
        }
    };
}
