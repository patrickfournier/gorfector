#pragma once

#include "DeviceSelectorState.hpp"
#include "ZooFW/Application.hpp"
#include "ZooFW/CommandDispatcher.hpp"
#include "ViewUpdateObserver.hpp"

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

        gulong m_DropdownSelectedSignalId;

        void OnRefreshDevicesClicked(GtkWidget *);
        void OnDeviceSelected(GtkWidget *);
        void OnActivateNetwork(GtkWidget *);

    public:
        DeviceSelector(Zoo::CommandDispatcher* parent, Zoo::Application* app);

        ~DeviceSelector();

        GtkWidget *RootWidget()
        { return m_DeviceSelectorRoot; }

        void Update(const DeviceSelectorState *stateComponent);
    };

}
