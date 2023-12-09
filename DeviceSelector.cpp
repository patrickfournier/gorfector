#include <stdexcept>
#include "DeviceSelector.h"
#include "ZooFW/SignalSupport.h"
#include "Commands/DeviceSelectorCommands.h"
#include "Commands/SelectDeviceCommand.h"
#include "ZooFW/ErrorDialog.h"


ZooScan::DeviceSelector::DeviceSelector(Zoo::CommandDispatcher* parent, Zoo::Application* app)
: m_App(app)
, m_Dispatcher(parent)
{
    m_State = new DeviceSelectorState();
    m_App->GetState()->AddStateComponent(m_State);
    m_Observer = new ViewUpdateObserver(this, m_State);
    m_App->GetObserverManager()->AddObserver(m_Observer);

    m_DeviceSelectorRoot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *deviceLabel = gtk_label_new("Device:");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), deviceLabel);

    const char* deviceList[] = {"No Devices Found", nullptr};
    m_DeviceSelectorList = gtk_drop_down_new_from_strings(deviceList);
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), m_DeviceSelectorList);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
    gtk_widget_set_sensitive(m_DeviceSelectorList, false);
    Zoo::ConnectGtkSignalWithParamSpecs(this, &DeviceSelector::OnDeviceSelected, m_DeviceSelectorList, "notify::selected");

    GtkWidget *refreshButton = gtk_button_new_with_label("Find Scanners");
    Zoo::ConnectGtkSignal(this, &DeviceSelector::OnRefreshDevicesClicked, refreshButton, "clicked");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), refreshButton);

    auto *networkScan = gtk_check_button_new_with_label("Network Scan");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), networkScan);
    Zoo::ConnectGtkSignal(this, &DeviceSelector::OnActivateNetwork, networkScan, "activate");

    m_Dispatcher.RegisterHandler<RefreshDeviceList, DeviceSelectorState>(RefreshDeviceList::Execute, m_State);
}

ZooScan::DeviceSelector::~DeviceSelector()
{
    m_Dispatcher.UnregisterHandler<RefreshDeviceList>();

    g_object_unref(m_DeviceSelectorRoot);

    m_App->GetObserverManager()->RemoveObserver(m_Observer);
    delete m_Observer;

    m_App->GetState()->RemoveStateComponent(m_State);
    delete m_State;
}

void ZooScan::DeviceSelector::OnRefreshDevicesClicked(GtkWidget*)
{
    m_Dispatcher.Dispatch(RefreshDeviceList());
}

void ZooScan::DeviceSelector::OnActivateNetwork(GtkWidget*)
{
    m_Dispatcher.Dispatch(ActivateNetworkScan());
}

void ZooScan::DeviceSelector::OnDeviceSelected(GtkWidget*)
{
    auto selectedIndex = gtk_drop_down_get_selected(GTK_DROP_DOWN(m_DeviceSelectorList));
    if (selectedIndex >= m_State->DeviceCount())
    {
        return;
    }

    auto device = m_State->DeviceList()[selectedIndex];
    m_Dispatcher.Dispatch(SelectDeviceCommand(device));
}

void ZooScan::DeviceSelector::Update(DeviceSelectorState *stateComponent)
{
    static uint64_t lastSeenVersion = 0;

    if (stateComponent != m_State)
    {
        throw std::runtime_error("State component mismatch");
    }

    if (m_State->Version() <= lastSeenVersion)
    {
        return;
    }

    auto deviceList = m_State->DeviceList();
    if (!deviceList.empty())
    {
        auto* devicesNames = new std::string[m_State->DeviceCount()];
        const char** deviceNameCStr = new const char*[m_State->DeviceCount() + 1];
        auto deviceIndex = 0U;
        while (deviceList[deviceIndex] != nullptr)
        {
            if (deviceIndex >= m_State->DeviceCount())
            {
                throw std::runtime_error("Device count mismatch");
            }

            std::string deviceName = deviceList[deviceIndex]->Vendor();
            deviceName += " ";
            deviceName += deviceList[deviceIndex]->Model();

            devicesNames[deviceIndex] = deviceName;
            deviceNameCStr[deviceIndex] = devicesNames[deviceIndex].c_str();

            deviceIndex++;
        }

        deviceNameCStr[deviceIndex] = nullptr;
        gtk_drop_down_set_model(GTK_DROP_DOWN(m_DeviceSelectorList), G_LIST_MODEL(gtk_string_list_new(deviceNameCStr)));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
        gtk_widget_set_sensitive(m_DeviceSelectorList, true);

        delete[] deviceNameCStr;
        delete[] devicesNames;
    }
    else
    {
        auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
        Zoo::ShowUserError(GTK_WINDOW(parentWindow), "No devices found"); // FIXME: Will not find devices powered on after app start

        const char* deviceListStr[] = {"No Devices Found", nullptr};
        gtk_drop_down_set_model(GTK_DROP_DOWN(m_DeviceSelectorList), G_LIST_MODEL(gtk_string_list_new(deviceListStr)));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
        gtk_widget_set_sensitive(m_DeviceSelectorList, false);
    }

    lastSeenVersion = m_State->Version();
}
