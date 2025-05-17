#pragma once

#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class LoadScanItemCommand
     * \brief Command class to load scan item settings into the application state.
     *
     * This class encapsulates the logic for applying scanner and output settings
     * to the respective application states (`DeviceOptionsState` and `OutputOptionsState`).
     */
    class LoadScanItemCommand : public ZooLib::Command
    {
        /**
         * \brief Pointer to the JSON object containing scanner settings.
         */
        const nlohmann::json *m_ScannerSettings;

        /**
         * \brief Pointer to the JSON object containing output settings.
         */
        const nlohmann::json *m_OutputSettings;

        /**
         * \brief Pointer to the JSON object containing scan area settings.
         */
        const nlohmann::json *m_ScanAreaSettings;

    public:
        /**
         * \brief Constructor for the LoadScanItemCommand.
         * \param scannerSettings Pointer to the JSON object with scanner settings.
         * \param outputSettings Pointer to the JSON object with output settings.
         * \param scanAreaSettings Pointer to the JSON object with scan area settings.
         */
        explicit LoadScanItemCommand(
                const nlohmann::json *scannerSettings, const nlohmann::json *outputSettings,
                const nlohmann::json *scanAreaSettings)
            : m_ScannerSettings(scannerSettings)
            , m_OutputSettings(outputSettings)
            , m_ScanAreaSettings(scanAreaSettings)
        {
        }

        /**
         * \brief Executes the command to apply the scan item settings.
         * \param command The `LoadScanItemCommand` instance containing the settings.
         * \param deviceOptions Pointer to the `DeviceOptionsState` where scanner settings will be applied.
         * \param outputOptions Pointer to the `OutputOptionsState` where output settings will be applied.
         */
        static void
        Execute(const LoadScanItemCommand &command, DeviceOptionsState *deviceOptions,
                OutputOptionsState *outputOptions)
        {
            if (command.m_ScannerSettings != nullptr && !command.m_ScannerSettings->empty())
            {
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplySettings(*command.m_ScannerSettings);
            }
            if (command.m_OutputSettings != nullptr && !command.m_OutputSettings->empty())
            {
                auto outputOptionsUpdater = OutputOptionsState::Updater(outputOptions);
                outputOptionsUpdater.ApplySettings(*command.m_OutputSettings);
            }
            if (command.m_ScanAreaSettings != nullptr && !command.m_ScanAreaSettings->empty())
            {
                auto deviceOptionsUpdater = DeviceOptionsState::Updater(deviceOptions);
                deviceOptionsUpdater.ApplyScanArea(*command.m_ScanAreaSettings);
            }
        }
    };
}
