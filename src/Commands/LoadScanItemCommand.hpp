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

    public:
        /**
         * \brief Constructor for the LoadScanItemCommand.
         * \param scannerSettings Pointer to the JSON object with scanner settings.
         * \param outputSettings Pointer to the JSON object with output settings.
         */
        explicit LoadScanItemCommand(const nlohmann::json *scannerSettings, const nlohmann::json *outputSettings)
            : m_ScannerSettings(scannerSettings)
            , m_OutputSettings(outputSettings)
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
