#pragma once

#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class DeviceOptionsState;
    class OutputOptionsState;

    /**
     * \class CreateScanListItemCommand
     * \brief Command class to create and add a new scan list item to the `ScanListState`.
     *
     * This class encapsulates the logic for creating a new scan list item using
     * device and output options and adding it to the `ScanListState`.
     */
    class CreateScanListItemCommand : public ZooLib::Command
    {
        /**
         * \brief Pointer to the device options state used to create the scan list item.
         */
        const DeviceOptionsState *m_DeviceOptions;

        /**
         * \brief Pointer to the output options state used to create the scan list item.
         */
        const OutputOptionsState *m_OutputOptions;

    public:
        /**
         * \brief Constructor for the CreateScanListItemCommand.
         * \param deviceOptions Pointer to the device options state.
         * \param outputOptions Pointer to the output options state.
         */
        CreateScanListItemCommand(const DeviceOptionsState *deviceOptions, const OutputOptionsState *outputOptions)
            : m_DeviceOptions(deviceOptions)
            , m_OutputOptions(outputOptions)
        {
        }

        /**
         * \brief Executes the command to add a new scan list item to the `ScanListState`.
         * \param command The CreateScanListItemCommand instance containing the options.
         * \param scanListState Pointer to the `ScanListState` where the item will be added.
         */
        static void Execute(const CreateScanListItemCommand &command, ScanListState *scanListState)
        {
            auto updater = ScanListState::Updater(scanListState);
            updater.AddScanItem(command.m_DeviceOptions, command.m_OutputOptions);
        }
    };
}
