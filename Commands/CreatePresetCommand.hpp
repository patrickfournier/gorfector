#pragma once
#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"
#include "ZooLib/json/single_include/nlohmann/json.hpp"

namespace ZooScan
{

    class CreatePresetCommand : public ZooLib::Command
    {
        nlohmann::json m_Preset{};

    public:
        explicit CreatePresetCommand(const nlohmann::json &j)
            : m_Preset(j)
        {
        }

        static void Execute(const CreatePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.AddPreset(command.m_Preset);
        }
    };

}
