#pragma once

#include "DeviceSelectorState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class ActivateNetworkLookUp
     * \brief Command class to activate or deactivate network lookup in the `DeviceSelectorState`.
     *
     * This class encapsulates the logic for toggling the network lookup functionality
     * in the `DeviceSelectorState`.
     */
    class ActivateNetworkLookUp : public ZooLib::Command
    {
        /**
         * \brief Flag indicating whether to enable or disable network lookup.
         */
        bool m_LookUpNetwork{};

    public:
        /**
         * \brief Constructor for the `ActivateNetworkLookUp` command.
         * \param activate The desired state of the network lookup flag.
         */
        explicit ActivateNetworkLookUp(bool activate)
            : m_LookUpNetwork(activate)
        {
        }

        /**
         * \brief Executes the command to update the network lookup setting.
         * \param command The `ActivateNetworkLookUp` instance containing the desired setting.
         * \param deviceSelectorState Pointer to the `DeviceSelectorState` where the setting will be updated.
         */
        static void Execute(const ActivateNetworkLookUp &command, DeviceSelectorState *deviceSelectorState)
        {
            DeviceSelectorState::Updater updater(deviceSelectorState);
            updater.SetLookUpNetwork(command.m_LookUpNetwork);
        }
    };
}
