#pragma once
#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace ZooScan
{

    class RenamePresetCommand : public ZooLib::Command
    {
        std::string m_PresetId{};
        std::string m_NewName{};

    public:
        explicit RenamePresetCommand(const std::string &presetId, const std::string &newName)
            : m_PresetId(presetId)
            , m_NewName(newName)
        {
        }

        static void Execute(const RenamePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.RenamePreset(command.m_PresetId, command.m_NewName);
        }
    };

}
