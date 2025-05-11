#pragma once

#include "ScanListState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class DeleteScanItemCommand
     * \brief Command class to delete a scan item from the `ScanListState`.
     *
     * This class encapsulates the logic for removing a scan item identified by its index
     * from the `ScanListState`.
     */
    class DeleteScanItemCommand : public ZooLib::Command
    {
        /**
         * \brief The index of the scan item to be deleted.
         */
        size_t m_Index{};

    public:
        /**
         * \brief Constructor for the DeleteScanItemCommand.
         * \param index The index of the scan item to be deleted.
         */
        explicit DeleteScanItemCommand(size_t index)
            : m_Index(index)
        {
        }

        /**
         * \brief Executes the command to delete the scan item.
         * \param command The DeleteScanItemCommand instance containing the index.
         * \param scanListState Pointer to the `ScanListState` where the scan item will be removed.
         */
        static void Execute(const DeleteScanItemCommand &command, ScanListState *scanListState)
        {
            auto updater = ScanListState::Updater(scanListState);
            updater.RemoveScanItemAt(command.m_Index);
        }
    };
}
