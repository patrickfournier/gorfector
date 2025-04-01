#include <c++/11/memory>
#include <stdexcept>

#include "Commands/DeviceSelectorCommands.hpp"
#include "Commands/SelectDeviceCommand.hpp"
#include "DeviceSelector.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/SignalSupport.hpp"


ZooScan::DeviceSelector::DeviceSelector(ZooLib::CommandDispatcher *parent, ZooLib::Application *app)
    : m_App(app)
    , m_Dispatcher(parent)
{
    m_State = new DeviceSelectorState(m_App->GetState());
    m_Observer = new ViewUpdateObserver(this, m_State);
    m_App->GetObserverManager()->AddObserver(m_Observer);

    m_DeviceSelectorRoot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *deviceLabel = gtk_label_new("Device:");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), deviceLabel);

    const char *deviceList[] = {"No Devices Found", nullptr};
    m_DeviceSelectorList = gtk_drop_down_new_from_strings(deviceList);
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), m_DeviceSelectorList);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
    gtk_widget_set_sensitive(m_DeviceSelectorList, false);
    m_DropdownSelectedSignalId = ZooLib::ConnectGtkSignalWithParamSpecs(
            this, &DeviceSelector::OnDeviceSelected, m_DeviceSelectorList, "notify::selected");

    GtkWidget *refreshButton = gtk_button_new_with_label("Find Scanners");
    ZooLib::ConnectGtkSignal(this, &DeviceSelector::OnRefreshDevicesClicked, refreshButton, "clicked");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), refreshButton);

    auto *networkScan = gtk_check_button_new_with_label("Network Scan");
    gtk_box_append(GTK_BOX(m_DeviceSelectorRoot), networkScan);
    ZooLib::ConnectGtkSignal(this, &DeviceSelector::OnActivateNetwork, networkScan, "toggled");

    m_Dispatcher.RegisterHandler<SelectDeviceCommand, DeviceSelectorState>(SelectDeviceCommand::Execute, m_State);
    m_Dispatcher.RegisterHandler<RefreshDeviceList, DeviceSelectorState>(RefreshDeviceList::Execute, m_State);
    m_Dispatcher.RegisterHandler<ActivateNetworkScan, DeviceSelectorState>(ActivateNetworkScan::Execute, m_State);
}

ZooScan::DeviceSelector::~DeviceSelector()
{
    m_Dispatcher.UnregisterHandler<SelectDeviceCommand>();
    m_Dispatcher.UnregisterHandler<RefreshDeviceList>();
    m_Dispatcher.UnregisterHandler<ActivateNetworkScan>();

    gtk_widget_unparent(m_DeviceSelectorRoot);

    m_App->GetObserverManager()->RemoveObserver(m_Observer);
    delete m_Observer;

    delete m_State;
}

void ZooScan::DeviceSelector::OnRefreshDevicesClicked(GtkWidget *)
{
    m_Dispatcher.Dispatch(RefreshDeviceList());
}

void ZooScan::DeviceSelector::OnActivateNetwork(GtkWidget *checkButton)
{
    const bool value = gtk_check_button_get_active(GTK_CHECK_BUTTON(checkButton));
    auto command = ActivateNetworkScan(value);
    m_Dispatcher.Dispatch(command);
}

void ZooScan::DeviceSelector::SelectDevice(const int deviceIndex)
{
    SaneDevice *device = nullptr;
    if (deviceIndex >= 0 && deviceIndex < static_cast<int>(m_State->GetDeviceList().size()))
    {
        device = m_State->GetDeviceList()[deviceIndex];
    }

    m_Dispatcher.Dispatch(
            SelectDeviceCommand(device == nullptr ? DeviceSelectorState::k_NullDeviceName : device->Name()));
}

void ZooScan::DeviceSelector::OnDeviceSelected(GtkWidget *)
{
    const auto selectedIndex = static_cast<int>(gtk_drop_down_get_selected(GTK_DROP_DOWN(m_DeviceSelectorList))) - 1;
    SelectDevice(selectedIndex);
}

void ZooScan::DeviceSelector::Update(u_int64_t lastSeenVersion) const
{
    g_signal_handler_block(m_DeviceSelectorList, m_DropdownSelectedSignalId);

    if (const auto deviceList = m_State->GetDeviceList(); !deviceList.empty())
    {
        const auto deviceCount = m_State->GetDeviceList().size();
        std::unique_ptr<std::string[]> devicesNames(new std::string[deviceCount + 1]);
        std::unique_ptr<const char *[]> deviceNamesCStr(new const char *[deviceCount + 2]);
        auto deviceIndex = 0U;
        auto dropDownItemIndex = 0U;
        auto selectedItemIndex = 0U;

        devicesNames[dropDownItemIndex] = "Select Device";
        deviceNamesCStr[dropDownItemIndex] = devicesNames[dropDownItemIndex].c_str();
        dropDownItemIndex++;

        while (deviceIndex < deviceCount)
        {
            const auto device = deviceList[deviceIndex];

            if (device == nullptr)
            {
                deviceIndex++;
                continue;
            }

            if (device->Name() == m_State->GetSelectedDeviceName())
            {
                selectedItemIndex = deviceIndex + 1;
            }

            std::string deviceName = device->Vendor();
            deviceName += " ";
            deviceName += device->Model();

            devicesNames[dropDownItemIndex] = deviceName;
            deviceNamesCStr[dropDownItemIndex] = devicesNames[dropDownItemIndex].c_str();

            deviceIndex++;
            dropDownItemIndex++;
        }

        deviceNamesCStr[dropDownItemIndex] = nullptr;
        try
        {
            gtk_drop_down_set_model(
                    GTK_DROP_DOWN(m_DeviceSelectorList), G_LIST_MODEL(gtk_string_list_new(deviceNamesCStr.get())));
            gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), selectedItemIndex);
            gtk_widget_set_sensitive(m_DeviceSelectorList, true);
        }
        catch (const SaneException &)
        {
            const auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
            ZooLib::ShowUserError(GTK_WINDOW(parentWindow), "Cannot open device %s.", deviceNamesCStr[0]);
        }
    }
    else
    {
        const auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
        ZooLib::ShowUserError(GTK_WINDOW(parentWindow), "No devices found.");

        const char *deviceListStr[] = {"No Devices Found", nullptr};
        gtk_drop_down_set_model(GTK_DROP_DOWN(m_DeviceSelectorList), G_LIST_MODEL(gtk_string_list_new(deviceListStr)));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(m_DeviceSelectorList), 0);
        gtk_widget_set_sensitive(m_DeviceSelectorList, false);
    }

    g_signal_handler_unblock(m_DeviceSelectorList, m_DropdownSelectedSignalId);
}
