#pragma once

#include <utility>

#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class RenamePresetCommand
     * \brief Command class to rename a preset in the `PresetPanelState`.
     *
     * This class encapsulates the logic for renaming a preset identified by its ID
     * in the `PresetPanelState`.
     */
    class RenamePresetCommand : public ZooLib::Command
    {
        /**
         * \brief The ID of the preset to be renamed.
         */
        std::string m_PresetId{};

        /**
         * \brief The new name for the preset.
         */
        std::string m_NewName{};

    public:
        /**
         * \brief Constructor for the RenamePresetCommand.
         * \param presetId The ID of the preset to be renamed.
         * \param newName The new name for the preset.
         */
        explicit RenamePresetCommand(std::string presetId, std::string newName)
            : m_PresetId(std::move(presetId))
            , m_NewName(std::move(newName))
        {
        }

        /**
         * \brief Executes the command to rename the preset.
         * \param command The `RenamePresetCommand` instance containing the preset ID and new name.
         * \param state Pointer to the `PresetPanelState` where the preset will be renamed.
         */
        static void Execute(const RenamePresetCommand &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.RenamePreset(command.m_PresetId, command.m_NewName);
        }
    };
}
