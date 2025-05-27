#pragma once

#include <utility>

#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class UpdatePresetCommand
     * \brief Command class to update a preset in the `PresetPanelState`.
     *
     * This class encapsulates the logic for updating a preset identified by its ID
     * in the `PresetPanelState`.
     */
    class UpdatePresetCommand : public ZooLib::Command
    {
        /**
         * \brief The ID of the preset to be updated.
         */
        std::string m_PresetId{};

        /**
         * \brief The new name for the preset.
         */
        std::string m_NewName{};

        /**
         * \brief JSON object containing the new values for the preset.
         */
        nlohmann::json m_NewValues{};

    public:
        /**
         * \brief Constructor for the UpdatePresetCommand.
         * \param presetId The ID of the preset to be updated.
         * \param newName The new name for the preset.
         * \param newValues JSON object containing the new values for the preset.
         */
        explicit UpdatePresetCommand(std::string presetId, std::string newName, nlohmann::json newValues)
            : m_PresetId(std::move(presetId))
            , m_NewName(std::move(newName))
            , m_NewValues(std::move(newValues))
        {
        }

        /**
         * \brief Executes the command to rename the preset.
         * \param command The `UpdatePresetCommand` instance containing the preset ID and update information.
         * \param state Pointer to the `PresetPanelState` where the preset will be updated.
         */
        static void Execute(const UpdatePresetCommand &command, PresetPanelState *state)
        {
            bool doRename = false;
            auto presetName =
                    state->GetPreset(command.m_PresetId)->at(PresetPanelState::k_PresetNameKey).get<std::string>();
            if (presetName != command.m_NewName)
            {
                doRename = true;
            }

            bool doUpdate = command.m_NewValues.contains(PresetPanelState::k_ScanAreaKey) ||
                            command.m_NewValues.contains(PresetPanelState::k_ScannerSettingsKey) ||
                            command.m_NewValues.contains(PresetPanelState::k_OutputSettingsKey);
            if (!doRename && !doUpdate)
            {
                return;
            }

            auto updater = PresetPanelState::Updater(state);
            if (doUpdate)
            {
                // Update first, so that preset id is not changed by a possible rename.
                updater.UpdatePreset(command.m_PresetId, command.m_NewValues);
            }
            if (doRename)
            {
                updater.RenamePreset(command.m_PresetId, command.m_NewName);
            }
        }
    };
}
