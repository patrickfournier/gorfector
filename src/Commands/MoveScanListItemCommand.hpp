#pragma once

#include "Command.hpp"
#include "ScanListState.hpp"

namespace Gorfector
{
    /**
     * \class MoveScanListItemCommand
     * \brief Command class to move a scan item within the `ScanListState`.
     *
     * This class encapsulates the logic for changing the position of a scan item
     * in the list managed by `ScanListState`.
     */
    class MoveScanListItemCommand : public ZooLib::Command
    {
        /**
         * \brief The index of the scan item to be moved.
         */
        int m_ItemIndex{};

        /**
         * \brief The change in position for the scan item.
         * A positive value moves the item down, a negative value moves it up.
         */
        int m_PositionDelta{};

    public:
        /**
         * \brief Constructor for the MoveScanListItemCommand.
         * \param itemIndex The index of the scan item to move.
         * \param positionDelta The amount to move the item by (positive for down, negative for up).
         */
        MoveScanListItemCommand(int itemIndex, int positionDelta)
            : m_ItemIndex(itemIndex)
            , m_PositionDelta(positionDelta)
        {
        }

        /**
         * \brief Executes the command to move the scan item.
         * \param command The MoveScanListItemCommand instance containing the item index and position delta.
         * \param scanListState Pointer to the `ScanListState` where the scan item will be moved.
         */
        static void Execute(const MoveScanListItemCommand &command, ScanListState *scanListState)
        {
            auto updater = ScanListState::Updater(scanListState);
            updater.MoveScanListItem(command.m_ItemIndex, command.m_PositionDelta);
        }
    };
}
