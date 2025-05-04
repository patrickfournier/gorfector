#pragma once

#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class ApplyPresetCommand
     * \brief Command class to apply a preset configuration to device and output options.
     *
     * This class is responsible for applying scanner settings, scan area settings,
     * and output settings from a given preset (in JSON format) to the respective
     * `DeviceOptionsState` and `OutputOptionsState` objects.
     *
     * \note The preset is expected to follow a specific structure with keys defined
     *       in `PresetPanelState`.
     */
    class ApplyPresetCommand : public ZooLib::Command
    {
        /**
         * \brief Pointer to the JSON object containing the preset configuration.
         */
        const nlohmann::json *m_Preset;

    public:
        /**
         * \brief Constructor for the ApplyPresetCommand.
         * \param preset Pointer to the JSON object containing the preset configuration.
         */
        explicit ApplyPresetCommand(const nlohmann::json *preset)
            : m_Preset(preset)
        {
        }

        /**
         * \brief Executes the command to apply the preset settings.
         *
         * This method applies the scanner settings, scan area settings, and output
         * settings from the preset to the provided `DeviceOptionsState` and
         * `OutputOptionsState` objects.
         *
         * \param command The ApplyPresetCommand instance containing the preset.
         * \param deviceOptions Pointer to the `DeviceOptionsState` to apply scanner settings and scan area.
         * \param outputOptions Pointer to the `OutputOptionsState` to apply output settings.
         */
        static void
        Execute(const ApplyPresetCommand &command, DeviceOptionsState *deviceOptions, OutputOptionsState *outputOptions)
        {
            if (command.m_Preset == nullptr)
            {
                return;
            }

            if (command.m_Preset->contains(PresetPanelState::k_ScannerSettingsKey))
            {
                auto scannerSettings = command.m_Preset->at(PresetPanelState::k_ScannerSettingsKey);
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplySettings(scannerSettings);
            }
            if (command.m_Preset->contains(PresetPanelState::k_ScanAreaKey))
            {
                auto scanAreaSettings = command.m_Preset->at(PresetPanelState::k_ScanAreaKey);
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplyScanArea(scanAreaSettings);
            }
            if (command.m_Preset->contains(PresetPanelState::k_OutputSettingsKey))
            {
                auto outputSettings = command.m_Preset->at(PresetPanelState::k_OutputSettingsKey);
                auto outputOptionsUpdater = OutputOptionsState::Updater(outputOptions);
                outputOptionsUpdater.ApplySettings(outputSettings);
            }
        }
    };
}
