#pragma once
#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    class DeleteScanItemCommand : public ZooLib::Command
    {
        size_t m_Index{};

    public:
        explicit DeleteScanItemCommand(size_t index)
            : m_Index(index)
        {
        }

        static void Execute(const DeleteScanItemCommand &command, ScanListState *scanListState)
        {
            auto updater = ScanListState::Updater(scanListState);
            updater.RemoveScanItemAt(command.m_Index);
        }
    };
}
