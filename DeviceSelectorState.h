#pragma once

#include "ZooFW/StateComponent.h"
#include "SaneDevice.h"

namespace ZooScan
{
    class DeviceSelectorState : public Zoo::StateComponent
    {
        std::vector<SaneDevice*> m_DeviceList{};
        bool m_ScanNetwork{};

        void GetDevicesFromSANE()
        {
            const SANE_Device **deviceList;
            SANE_Status status = sane_get_devices(&deviceList, ScanNetwork() ? SANE_FALSE : SANE_TRUE);
            if (status == SANE_STATUS_GOOD)
            {
                auto deviceCount = 0;
                while (deviceList[deviceCount] != nullptr)
                {
                    deviceCount++;
                }

                m_DeviceList.clear();
                m_DeviceList.reserve(deviceCount);
                for (int i = 0; i < deviceCount; i++)
                {
                    m_DeviceList.push_back(new SaneDevice(deviceList[i]));
                }
            }
        }

    public:
        [[nodiscard]] const std::vector<SaneDevice*> &DeviceList() const
        { return m_DeviceList; }

        [[nodiscard]] bool ScanNetwork() const
        { return m_ScanNetwork; }

        explicit DeviceSelectorState(Zoo::State* state)
        : StateComponent(state)
        {
            GetDevicesFromSANE();
        }

        ~DeviceSelectorState() override
        {
            for (auto device : m_DeviceList)
            {
                delete device;
            }
        }

        class Updater : public Zoo::StateComponent::Updater<DeviceSelectorState>
        {
        public:
            explicit Updater(DeviceSelectorState *state)
                    : StateComponent::Updater<DeviceSelectorState>(state)
            {}

            void UpdateDeviceList()
            {
                m_StateComponent->GetDevicesFromSANE();
            }

            void SetScanNetwork(bool scanNetwork)
            {
                m_StateComponent->m_ScanNetwork = scanNetwork;
            }
        };
    };
}
