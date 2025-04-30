#pragma once
#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class LoadScanItemCommand : public ZooLib::Command
    {
        const nlohmann::json *m_ScannerSettings;
        const nlohmann::json *m_OutputSettings;

    public:
        explicit LoadScanItemCommand(const nlohmann::json *scannerSettings, const nlohmann::json *outputSettings)
            : m_ScannerSettings(scannerSettings)
            , m_OutputSettings(outputSettings)
        {
        }

        static void
        Execute(const LoadScanItemCommand &command, DeviceOptionsState *deviceOptions,
                OutputOptionsState *outputOptions)
        {
            if (!command.m_ScannerSettings->empty())
            {
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplySettings(*command.m_ScannerSettings);
            }
            if (!command.m_OutputSettings->empty())
            {
                auto outputOptionsUpdater = OutputOptionsState::Updater(outputOptions);
                outputOptionsUpdater.ApplySettings(*command.m_OutputSettings);
            }
        }
    };
}
