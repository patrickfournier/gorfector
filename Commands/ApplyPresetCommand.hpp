#pragma once
#include <string>


#include "DeviceOptionsState.hpp"

#include "OutputOptionsState.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class ApplyPresetCommand : public ZooLib::Command
    {
        const nlohmann::json *m_Preset;

    public:
        explicit ApplyPresetCommand(const nlohmann::json *preset)
            : m_Preset(preset)
        {
        }

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
