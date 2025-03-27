#include <stdexcept>
#include <c++/11/memory>
#include "DeviceSelector.hpp"
#include "ZooFW/SignalSupport.hpp"
#include "Commands/DeviceSelectorCommands.hpp"
#include "Commands/SelectDeviceCommand.hpp"
#include "ZooFW/ErrorDialog.hpp"


ZooScan::DeviceSelector::DeviceSelector(Zoo::CommandDispatcher* parent, Zoo::Application* app)
: m_App(app)
, m_Dispatcher(parent)
{
    m_State = new DeviceSelectorState(m_App->GetState());
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
    m_DropdownSelectedSignalId = Zoo::ConnectGtkSignalWithParamSpecs(this, &DeviceSelector::OnDeviceSelected, m_DeviceSelectorList, "notify::selected");

    GtkWidget *refreshButton = gtk_button_new_with_label("Find Scanners");
    Zoo::ConnectGtkSignal(this, &DeviceSelector::OnRefreshDevicesClicked, refreshButton, "clicked");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), refreshButton);

    auto *networkScan = gtk_check_button_new_with_label("Network Scan");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), networkScan);
    Zoo::ConnectGtkSignal(this, &DeviceSelector::OnActivateNetwork, networkScan, "toggled");

    m_Dispatcher.RegisterHandler<RefreshDeviceList, DeviceSelectorState>(RefreshDeviceList::Execute, m_State);
    m_Dispatcher.RegisterHandler<ActivateNetworkScan, DeviceSelectorState>(ActivateNetworkScan::Execute, m_State);
}

ZooScan::DeviceSelector::~DeviceSelector()
{
    m_Dispatcher.UnregisterHandler<RefreshDeviceList>();

    gtk_widget_unparent(m_DeviceSelectorRoot);

    m_App->GetObserverManager()->RemoveObserver(m_Observer);
    delete m_Observer;

    delete m_State;
}

void ZooScan::DeviceSelector::OnRefreshDevicesClicked(GtkWidget*)
{
    m_Dispatcher.Dispatch(RefreshDeviceList());
}

void ZooScan::DeviceSelector::OnActivateNetwork(GtkWidget* checkButton)
{
    bool value = gtk_check_button_get_active(GTK_CHECK_BUTTON(checkButton));
    auto command = ActivateNetworkScan();
    command.ScanNetwork = value;
    m_Dispatcher.Dispatch(command);
}

void ZooScan::DeviceSelector::OnDeviceSelected(GtkWidget*)
{
    auto selectedIndex = int(gtk_drop_down_get_selected(GTK_DROP_DOWN(m_DeviceSelectorList))) - 1;

    SaneDevice* device = nullptr;
    if (selectedIndex < int(m_State->DeviceList().size()))
    {
        device = m_State->DeviceList()[selectedIndex];
    }

    m_Dispatcher.Dispatch(SelectDeviceCommand(device));
}

void ZooScan::DeviceSelector::Update(const DeviceSelectorState *stateComponent)
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

    g_signal_handler_block(m_DeviceSelectorList, m_DropdownSelectedSignalId);

    auto deviceList = m_State->DeviceList();
    if (!deviceList.empty())
    {
        auto deviceCount = m_State->DeviceList().size();
        std::unique_ptr<std::string[]> devicesNames(new std::string[deviceCount + 1]);
        std::unique_ptr<const char*[]> deviceNamesCStr(new const char*[deviceCount + 2]);
        auto deviceIndex = 0U;

        devicesNames[deviceIndex] = "Select Device";
        deviceNamesCStr[deviceIndex] = devicesNames[deviceIndex].c_str();
        deviceIndex++;

        while (deviceIndex - 1 < deviceCount)
        {
            std::string deviceName = deviceList[deviceIndex - 1]->Vendor();
            deviceName += " ";
            deviceName += deviceList[deviceIndex - 1]->Model();

            devicesNames[deviceIndex] = deviceName;
            deviceNamesCStr[deviceIndex] = devicesNames[deviceIndex].c_str();

            deviceIndex++;
        }

        deviceNamesCStr[deviceIndex] = nullptr;
        try
        {
            gtk_drop_down_set_model(GTK_DROP_DOWN(m_DeviceSelectorList),
                                    G_LIST_MODEL(gtk_string_list_new(deviceNamesCStr.get())));
            gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
            gtk_widget_set_sensitive(m_DeviceSelectorList, true);
        }
        catch (const SaneException& e)
        {
            auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
            Zoo::ShowUserError(GTK_WINDOW(parentWindow), "Cannot open device %s.", deviceNamesCStr[0]);
        }
    }
    else
    {
        auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
        Zoo::ShowUserError(GTK_WINDOW(parentWindow), "No devices found.");

        const char* deviceListStr[] = {"No Devices Found", nullptr};
        gtk_drop_down_set_model(GTK_DROP_DOWN(m_DeviceSelectorList), G_LIST_MODEL(gtk_string_list_new(deviceListStr)));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
        gtk_widget_set_sensitive(m_DeviceSelectorList, false);
    }

    g_signal_handler_unblock(m_DeviceSelectorList, m_DropdownSelectedSignalId);

    lastSeenVersion = m_State->Version();
}
