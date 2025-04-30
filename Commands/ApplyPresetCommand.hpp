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

            if (command.m_Preset->contains(PresetPanelState::s_PresetSectionScannerSettings))
            {
                auto scannerSettings = command.m_Preset->at(PresetPanelState::s_PresetSectionScannerSettings);
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplySettings(scannerSettings);
            }
            if (command.m_Preset->contains(PresetPanelState::s_PresetSectionScanArea))
            {
                auto scanAreaSettings = command.m_Preset->at(PresetPanelState::s_PresetSectionScanArea);
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplyScanArea(scanAreaSettings);
            }
            if (command.m_Preset->contains(PresetPanelState::s_PresetSectionOutputSettings))
            {
                auto outputSettings = command.m_Preset->at(PresetPanelState::s_PresetSectionOutputSettings);
                auto outputOptionsUpdater = OutputOptionsState::Updater(outputOptions);
                // outputOptionsUpdater.ApplySettings(outputSettings);
            }
        }
    };
}
