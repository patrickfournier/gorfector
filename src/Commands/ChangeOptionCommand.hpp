#pragma once

#include "DeviceOptionsState.hpp"
#include "ZooLib/Command.hpp"

namespace Gorfector
{
    /**
     * \class ChangeOptionCommand
     * \brief Template class to represent a command for changing an option in the device options state.
     *
     * This class encapsulates the logic for modifying a specific option in the `DeviceOptionsState`
     * using a setting index, value index, and the new value. It supports multiple value types
     * through template specialization.
     *
     * \tparam TValueType The type of the value to be set (e.g., `bool`, `int`, `std::string`).
     */
    template<typename TValueType>
    class ChangeOptionCommand : public ZooLib::Command
    {
        /**
         * \brief The index of the setting to be modified.
         */
        const uint32_t m_SettingIndex;

        /**
         * \brief The index of the value within the setting to be modified.
         */
        const uint32_t m_ValueIndex;

        /**
         * \brief The new value to be set.
         */
        const TValueType m_Value;

    public:
        /**
         * \brief Constructor for the ChangeOptionCommand.
         * \param settingIndex The index of the setting to be modified.
         * \param valueIndex The index of the value within the setting to be modified.
         * \param value The new value to be set.
         */
        ChangeOptionCommand(uint32_t settingIndex, uint32_t valueIndex, TValueType value)
            : m_SettingIndex(settingIndex)
            , m_ValueIndex(valueIndex)
            , m_Value(value)
        {
        }

        /**
         * \brief Executes the command for a `bool` value type.
         * \param command The command containing the setting and value information.
         * \param state Pointer to the `DeviceOptionsState` to be updated.
         */
        static void Execute(const ChangeOptionCommand<bool> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
        }

        /**
         * \brief Executes the command for an `int` value type.
         * \param command The command containing the setting and value information.
         * \param state Pointer to the `DeviceOptionsState` to be updated.
         */
        static void Execute(const ChangeOptionCommand<int> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
        }

        /**
         * \brief Executes the command for a `std::string` value type.
         * \param command The command containing the setting and value information.
         * \param state Pointer to the `DeviceOptionsState` to be updated.
         */
        static void Execute(const ChangeOptionCommand<std::string> &command, DeviceOptionsState *state)
        {
            auto updater = DeviceOptionsState::Updater(state);
            updater.SetOptionValue(command.m_SettingIndex, command.m_ValueIndex, command.m_Value);
        }
    };
}
