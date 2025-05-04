#pragma once

#include <string>
#include <utility>

#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class DeletePresetCommand
     * \brief Command class to delete a preset from the `PresetPanelState`.
     *
     * This class encapsulates the logic for removing a preset identified by its ID
     * from the `PresetPanelState`.
     */
    class DeletePresetCommand : public ZooLib::Command
    {
        /**
         * \brief The ID of the preset to be deleted.
         */
        std::string m_PresetId{};

    public:
        /**
         * \brief Constructor for the DeletePresetCommand.
         * \param presetId The ID of the preset to be deleted.
         */
        explicit DeletePresetCommand(std::string presetId)
            : m_PresetId(std::move(presetId))
        {
        }

        /**
         * \brief Executes the command to delete the preset.
         * \param command The DeletePresetCommand instance containing the preset ID.
         * \param state Pointer to the `PresetPanelState` where the preset will be removed.
         */
        static void Execute(const DeletePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.RemovePreset(command.m_PresetId);
        }
    };
}
