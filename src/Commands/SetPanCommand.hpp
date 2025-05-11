#pragma once

#include "PreviewState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetPanCommand
     * \brief Command class to set the pan offset in the `PreviewState`.
     *
     * This class encapsulates the logic for updating the pan offset
     * in the `PreviewState`.
     */
    class SetPanCommand : public ZooLib::Command
    {
        /**
         * \brief The desired pan offset.
         */
        const Point<double> m_PanOffset{};

    public:
        /**
         * \brief Constructor for the SetPanCommand.
         * \param panOffset The desired pan offset to set.
         */
        explicit SetPanCommand(const Point<double> panOffset)
            : m_PanOffset(panOffset)
        {
        }

        /**
         * \brief Executes the command to set the pan offset.
         * \param command The `SetPanCommand` instance containing the desired pan offset.
         * \param previewState Pointer to the `PreviewState` where the pan offset will be updated.
         */
        static void Execute(const SetPanCommand &command, PreviewState *previewState)
        {
            auto updater = PreviewState::Updater(previewState);
            updater.SetPanOffset(command.m_PanOffset);
        }
    };
}
