#pragma once

#include "DeviceSelectorState.h"
#include "ZooFW/Application.h"
#include "ZooFW/CommandDispatcher.h"
#include "ViewUpdateObserver.h"

namespace ZooScan
{
    class DeviceSelector
    {
        DeviceSelectorState *m_State;
        ViewUpdateObserver<DeviceSelector, DeviceSelectorState> *m_Observer;

        Zoo::Application *m_App;
        Zoo::CommandDispatcher m_Dispatcher{};

        GtkWidget *m_DeviceSelectorRoot{};
        GtkWidget *m_DeviceSelectorList{};

        void OnRefreshDevicesClicked(GtkWidget *);
        void OnDeviceSelected(GtkWidget *);
        void OnActivateNetwork(GtkWidget *);

    public:
        DeviceSelector(Zoo::CommandDispatcher* parent, Zoo::Application* app);

        ~DeviceSelector();

        GtkWidget *RootWidget()
        { return m_DeviceSelectorRoot; }

        void Update(DeviceSelectorState *stateComponent);
    };

}
