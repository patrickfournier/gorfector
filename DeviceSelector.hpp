#pragma once

#include "DeviceSelectorState.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/Application.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace ZooScan
{
    class DeviceSelector : public ZooLib::View
    {
        DeviceSelectorState *m_State;
        ViewUpdateObserver<DeviceSelector, DeviceSelectorState> *m_Observer;

        ZooLib::Application *m_App;
        ZooLib::CommandDispatcher m_Dispatcher{};

        GtkWidget *m_DeviceSelectorRoot{};
        GtkWidget *m_DeviceSelectorList{};

        gulong m_DropdownSelectedSignalId;

        void OnRefreshDevicesClicked(GtkWidget *);
        void OnDeviceSelected(GtkWidget *);
        void OnActivateNetwork(GtkWidget *);

        void SelectDevice(int deviceIndex);

    public:
        DeviceSelector(
                ZooLib::CommandDispatcher *parent, ZooLib::Application *app, DeviceSelectorState *deviceSelectorState);

        ~DeviceSelector() override;

        DeviceSelectorState *GetState() const
        {
            return m_State;
        }

        GtkWidget *GetRootWidget() const override
        {
            return m_DeviceSelectorRoot;
        }

        void Update(uint64_t lastSeenVersion) override;

        void SelectDefaultDevice()
        {
            SelectDevice(0);
        }
    };
}
