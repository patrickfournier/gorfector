#pragma once

#include <vector>

#include "OptionRewriter.hpp"
#include "SaneDevice.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    /**
     * \class DeviceSelectorState
     * \brief Manages the state of the device selector, including the list of devices and the selected device.
     *
     * The `DeviceSelectorState` class is responsible for maintaining the list of available devices,
     * tracking the currently selected device, and interacting with the SANE library to retrieve and manage devices.
     * It provides methods to update the device list, select a device, and configure network lookup settings.
     */
    class DeviceSelectorState final : public ZooLib::StateComponent
    {
    public:
        static const std::string k_NullDeviceName;

    private:
        std::vector<SaneDevice *> m_DeviceList{};
        std::string m_SelectedDeviceName{};
        bool m_NetworkLookUp{};
        int m_SANEInitId{};
        bool m_DumpSane{};

        /**
         * \brief Retrieves the list of devices from the SANE library.
         *
         * This method clears the current device list, reinitializes the SANE library, and fetches the latest
         * list of devices. It also ensures that the previously selected device is reselected if it is still available.
         */
        void GetDevicesFromSANE()
        {
            const auto currentlySelectedDeviceName = m_SelectedDeviceName;

            SelectDevice(k_NullDeviceName);
            for (const SaneDevice *const device: m_DeviceList)
            {
                delete device;
            }
            m_DeviceList.clear();

            // Although the SANE documentation states that sane_get_devices() can pick up newly available devices,
            // it does not seem to be the case. Re-init SANE to get the latest devices (this invalidates all previous
            // SANE data).
            SANE_Int saneVersion;
            sane_exit();
            if (SANE_STATUS_GOOD != sane_init(&saneVersion, nullptr))
            {
                sane_exit();
                return;
            }
            ++m_SANEInitId;

            const SANE_Device **deviceList;
            SANE_Status status = sane_get_devices(&deviceList, IsNetworkLookUpEnabled() ? SANE_FALSE : SANE_TRUE);
            if (status == SANE_STATUS_GOOD)
            {
                auto deviceCount = 0;
                while (deviceList[deviceCount] != nullptr)
                {
                    deviceCount++;
                }

                m_DeviceList.reserve(deviceCount);
                for (int i = 0; i < deviceCount; i++)
                {
                    if (deviceList[i] != nullptr)
                        m_DeviceList.push_back(new SaneDevice(deviceList[i]));

                    if (deviceList[i]->name == currentlySelectedDeviceName)
                    {
                        SelectDevice(currentlySelectedDeviceName);
                    }
                }
            }
        }

        /**
         * \brief Selects a device by its name.
         *
         * This method updates the currently selected device, closes the previously selected device (if any),
         * and opens the new device. If the specified device name is invalid, it resets the selection to a null device.
         *
         * \param deviceName The name of the device to select.
         */
        void SelectDevice(const std::string &deviceName)
        {
            if (m_SelectedDeviceName == deviceName)
                return;

            auto device = GetDeviceByName(m_SelectedDeviceName);
            if (device != nullptr)
                device->Close();

            device = GetDeviceByName(deviceName);

            m_SelectedDeviceName = k_NullDeviceName;
            if (device != nullptr)
            {
                if (device->Open())
                {
                    m_SelectedDeviceName = deviceName;
                }
            }

            if (m_DumpSane && (m_SelectedDeviceName != k_NullDeviceName))
            {
                OptionRewriter::Dump(GetDeviceByName(m_SelectedDeviceName));
            }
        }

    public:
        /**
         * \brief Retrieves the current SANE initialization ID.
         * \return The SANE initialization ID.
         */
        [[nodiscard]] int GetSelectorSaneInitId() const
        {
            return m_SANEInitId;
        }

        /**
         * \brief Retrieves the list of available devices.
         * \return A constant reference to the vector of `SaneDevice` pointers.
         */
        [[nodiscard]] const std::vector<SaneDevice *> &GetDeviceList() const
        {
            return m_DeviceList;
        }

        /**
         * \brief Retrieves a device by its name.
         * \param deviceName The name of the device to retrieve.
         * \return A pointer to the `SaneDevice` if found, or nullptr if not found.
         */
        [[nodiscard]] SaneDevice *GetDeviceByName(const std::string &deviceName) const
        {
            if (deviceName.empty())
                return nullptr;

            for (const auto device: m_DeviceList)
            {
                if (device->GetName() == deviceName)
                {
                    return device;
                }
            }
            return nullptr;
        }

        /**
         * \brief Retrieves the name of the currently selected device.
         * \return A constant reference to the selected device name.
         */
        [[nodiscard]] const std::string &GetSelectedDeviceName() const
        {
            return m_SelectedDeviceName;
        }

        /**
         * \brief Checks if network lookup is enabled.
         * \return True if network lookup is enabled, false otherwise.
         */
        [[nodiscard]] bool IsNetworkLookUpEnabled() const
        {
            return m_NetworkLookUp;
        }

        /**
         * \brief Enables or disables the dumping of SANE options.
         * \return True if dumping is enabled, false otherwise.
         */
        [[nodiscard]] bool IsDumpSaneEnabled() const
        {
            return m_DumpSane;
        }

        /**
         * \brief Constructs a `DeviceSelectorState` instance and initializes the device list.
         *
         * This constructor initializes the `DeviceSelectorState` by fetching the list of devices
         * from the SANE library and selecting the first available device if the list is not empty.
         *
         * \param state Pointer to the parent `ZooLib::State` instance.
         */
        explicit DeviceSelectorState(ZooLib::State *state)
            : StateComponent(state)
        {
            GetDevicesFromSANE();
            if (!m_DeviceList.empty())
            {
                SelectDevice(m_DeviceList[0]->GetName());
            }
        }

        /**
         * \brief Destructor for the `DeviceSelectorState` class.
         */
        ~DeviceSelectorState() override
        {
            for (const auto device: m_DeviceList)
            {
                delete device;
            }
        }

        /**
         * \class Updater
         * \brief Provides an interface to update the state of the `DeviceSelectorState`.
         *
         * The `Updater` class allows controlled modifications to the `DeviceSelectorState`,
         * such as updating the device list, selecting a device, enabling/disabling network lookup,
         * and toggling the dumping of SANE options.
         */
        class Updater final : public StateComponent::Updater<DeviceSelectorState>
        {
        public:
            /**
             * \brief Constructs an `Updater` for the given `DeviceSelectorState`.
             * \param state Pointer to the `DeviceSelectorState` to be updated.
             */
            explicit Updater(DeviceSelectorState *state)
                : StateComponent::Updater<DeviceSelectorState>(state)
            {
            }

            /**
             * \brief Loads the state from a JSON object (unimplemented).
             * \param json The JSON object containing the state data.
             */
            void LoadFromJson(const nlohmann::json &json) override
            {
            }

            /**
             * \brief Updates the device list by fetching the latest devices from the SANE library.
             */
            void UpdateDeviceList() const
            {
                m_StateComponent->GetDevicesFromSANE();
            }

            /**
             * \brief Enables or disables network lookup for devices.
             * \param networkLookUp True to enable network lookup, false to disable it.
             */
            void SetLookUpNetwork(const bool networkLookUp) const
            {
                m_StateComponent->m_NetworkLookUp = networkLookUp;
            }

            /**
             * \brief Selects a device by its name.
             * \param deviceName The name of the device to select.
             */
            void SelectDevice(const std::string &deviceName) const
            {
                m_StateComponent->SelectDevice(deviceName);
            }

            /**
             * \brief Enables or disables the dumping of SANE options for the selected device.
             * \param dumpSane True to enable dumping, false to disable it.
             */
            void SetDumpSaneOptions(const bool dumpSane) const
            {
                m_StateComponent->m_DumpSane = dumpSane;
            }
        };
    };
}
