#pragma once

#include <sane/sane.h>
#include "ZooFW/StateComponent.h"

namespace ZooScan
{
    class DeviceSelectorState : public Zoo::StateComponent
    {
        const SANE_Device **m_DeviceList{};
        int m_DeviceCount{};
        bool m_ScanNetwork{};

        void GetDevicesFromSANE()
        {
            const SANE_Device **deviceList;
            SANE_Status status = sane_get_devices(&deviceList, ScanNetwork() ? SANE_FALSE : SANE_TRUE);
            if (status == SANE_STATUS_GOOD)
            {
                m_DeviceList = deviceList;

                m_DeviceCount = 0;
                while (deviceList[m_DeviceCount] != nullptr)
                {
                    m_DeviceCount++;
                }
            }
        }

    public:
        [[nodiscard]] const SANE_Device **DeviceList() const
        { return m_DeviceList; }

        [[nodiscard]] bool ScanNetwork() const
        { return m_ScanNetwork; }

        [[nodiscard]] int DeviceCount() const
        { return m_DeviceCount; }

        DeviceSelectorState()
        {
            GetDevicesFromSANE();
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
