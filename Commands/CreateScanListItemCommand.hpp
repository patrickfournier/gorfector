#pragma once
#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class DeviceOptionsState;
    class OutputOptionsState;

    class CreateScanListItemCommand : public ZooLib::Command
    {
        const DeviceOptionsState *m_DeviceOptions;
        const OutputOptionsState *m_OutputOptions;

    public:
        CreateScanListItemCommand(const DeviceOptionsState *deviceOptions, const OutputOptionsState *outputOptions)
            : m_DeviceOptions(deviceOptions)
            , m_OutputOptions(outputOptions)
        {
        }

        static void Execute(const CreateScanListItemCommand &command, ScanListState *scanListState)
        {
            auto updater = ScanListState::Updater(scanListState);
            updater.AddScanItem(command.m_DeviceOptions, command.m_OutputOptions);
        }
    };
}
