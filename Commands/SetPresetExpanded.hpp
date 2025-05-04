#pragma once

#include "PresetPanelState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class SetPresetExpanded
     * \brief Command class to set the expanded state of the preset panel in the `PresetPanelState`.
     *
     * This class encapsulates the logic for updating the expanded state
     * of the preset panel in the `PresetPanelState`.
     */
    class SetPresetExpanded : public ZooLib::Command
    {
        /**
         * \brief The desired expanded state of the preset panel.
         */
        bool m_Expanded{};

    public:
        /**
         * \brief Constructor for the SetPresetExpanded command.
         * \param expanded The desired expanded state to set.
         */
        explicit SetPresetExpanded(bool expanded)
            : m_Expanded(expanded)
        {
        }

        /**
         * \brief Executes the command to set the expanded state of the preset panel.
         * \param command The `SetPresetExpanded` instance containing the desired state.
         * \param state Pointer to the `PresetPanelState` where the expanded state will be updated.
         */
        static void Execute(const SetPresetExpanded &command, PresetPanelState *state)
        {
            auto updater = PresetPanelState::Updater(state);
            updater.SetExpanded(command.m_Expanded);
        }
    };
}
