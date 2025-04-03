#pragma once

#include "DeviceSelectorState.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/Application.hpp"
#include "ZooLib/CommandDispatcher.hpp"

namespace ZooScan
{
    class DeviceSelector
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
        DeviceSelector(ZooLib::CommandDispatcher *parent, ZooLib::Application *app);

        ~DeviceSelector();

        DeviceSelectorState *GetState() const
        {
            return m_State;
        }

        GtkWidget *RootWidget() const
        {
            return m_DeviceSelectorRoot;
        }

        void Update(u_int64_t lastSeenVersion) const;

        void SelectDefaultDevice()
        {
            SelectDevice(0);
        }
    };
}
