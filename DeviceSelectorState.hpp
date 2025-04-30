#pragma once

#include <vector>

#include "OptionRewriter.hpp"
#include "SaneDevice.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    class DeviceSelectorState final : public ZooLib::StateComponent
    {
    public:
        static const std::string k_NullDeviceName;

    private:
        std::vector<SaneDevice *> m_DeviceList{};
        std::string m_SelectedDeviceName{};
        bool m_ScanNetwork{};
        int m_SANEInitId{};
        bool m_DumpSane{};

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
                throw std::runtime_error("Failed to initialize SANE");
            }
            ++m_SANEInitId;

            const SANE_Device **deviceList;
            SANE_Status status = sane_get_devices(&deviceList, IsScanNetworkEnabled() ? SANE_FALSE : SANE_TRUE);
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

        void SelectDevice(const std::string &deviceName)
        {
            if (m_SelectedDeviceName == deviceName)
                return;

            auto device = GetDeviceByName(m_SelectedDeviceName);
            if (device != nullptr)
                device->Close();

            device = GetDeviceByName(deviceName);
            if (device != nullptr)
            {
                m_SelectedDeviceName = deviceName;
                // FIXME: can throw
                device->Open();
            }
            else
            {
                m_SelectedDeviceName = k_NullDeviceName;
            }

            if (m_DumpSane && (m_SelectedDeviceName != k_NullDeviceName))
            {
                OptionRewriter::Dump(GetDeviceByName(m_SelectedDeviceName));
            }
        }

    public:
        [[nodiscard]] int GetSelectorSaneInitId() const
        {
            return m_SANEInitId;
        }

        [[nodiscard]] const std::vector<SaneDevice *> &GetDeviceList() const
        {
            return m_DeviceList;
        }

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

        [[nodiscard]] const std::string &GetSelectedDeviceName() const
        {
            return m_SelectedDeviceName;
        }

        [[nodiscard]] bool IsScanNetworkEnabled() const
        {
            return m_ScanNetwork;
        }

        explicit DeviceSelectorState(ZooLib::State *state)
            : StateComponent(state)
        {
            GetDevicesFromSANE();
            if (!m_DeviceList.empty())
            {
                SelectDevice(m_DeviceList[0]->GetName());
            }
        }

        ~DeviceSelectorState() override
        {
            for (const auto device: m_DeviceList)
            {
                delete device;
            }
        }

        class Updater final : public StateComponent::Updater<DeviceSelectorState>
        {
        public:
            explicit Updater(DeviceSelectorState *state)
                : StateComponent::Updater<DeviceSelectorState>(state)
            {
            }

            void LoadFromJson(const nlohmann::json &json) override
            {
            }

            void UpdateDeviceList() const
            {
                m_StateComponent->GetDevicesFromSANE();
            }

            void SetScanNetwork(const bool scanNetwork) const
            {
                m_StateComponent->m_ScanNetwork = scanNetwork;
            }

            void SelectDevice(const std::string &deviceName) const
            {
                m_StateComponent->SelectDevice(deviceName);
            }

            void SetDumpSaneOptions(const bool dumpSane) const
            {
                m_StateComponent->m_DumpSane = dumpSane;
            }
        };
    };
}
