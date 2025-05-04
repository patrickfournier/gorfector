#pragma once

#include <utility>

#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"
#include "ZooLib/json/single_include/nlohmann/json.hpp"

namespace Gorfector
{
    /**
     * \class CreatePresetCommand
     * \brief Command class to create and add a new preset to the `PresetPanelState`.
     *
     * This class encapsulates the logic for creating a new preset using a JSON object
     * and adding it to the `PresetPanelState`.
     */
    class CreatePresetCommand : public ZooLib::Command
    {
        /**
         * \brief JSON object representing the preset to be created.
         */
        nlohmann::json m_Preset{};

    public:
        /**
         * \brief Constructor for the CreatePresetCommand.
         * \param j JSON object containing the preset data.
         */
        explicit CreatePresetCommand(nlohmann::json j)
            : m_Preset(std::move(j))
        {
        }

        /**
         * \brief Executes the command to add the preset to the `PresetPanelState`.
         * \param command The CreatePresetCommand instance containing the preset data.
         * \param state Pointer to the `PresetPanelState` where the preset will be added.
         */
        static void Execute(const CreatePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.AddPreset(command.m_Preset);
        }
    };
}
