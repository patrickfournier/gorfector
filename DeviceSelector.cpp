#include <format>
#include <memory>

#include "Commands/DeviceSelectorCommands.hpp"
#include "Commands/SelectDeviceCommand.hpp"
#include "DeviceSelector.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/SignalSupport.hpp"


Gorfector::DeviceSelector::DeviceSelector(
        ZooLib::CommandDispatcher *parent, ZooLib::Application *app, DeviceSelectorState *deviceSelectorState)
    : m_App(app)
    , m_Dispatcher(parent)
{
    const char *listBoxClasses[] = {"boxed-list", nullptr};
    const char *buttonClasses[] = {"flat", nullptr};

    m_State = deviceSelectorState;
    m_Observer = new ViewUpdateObserver(this, m_State);
    m_App->GetObserverManager()->AddObserver(m_Observer);

    m_DeviceSelectorRoot = gtk_list_box_new();
    gtk_widget_set_margin_top(m_DeviceSelectorRoot, 20);
    gtk_widget_set_margin_bottom(m_DeviceSelectorRoot, 20);
    gtk_widget_set_margin_start(m_DeviceSelectorRoot, 20);
    gtk_widget_set_margin_end(m_DeviceSelectorRoot, 20);
    gtk_widget_set_css_classes(GTK_WIDGET(m_DeviceSelectorRoot), listBoxClasses);

    const char *deviceListNames[] = {_("No Scanner Found"), nullptr};
    auto *deviceList = gtk_string_list_new(deviceListNames);

    /* TODO add button with icon view-refresh-symbolic */
    m_DeviceSelectorList = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_DeviceSelectorList), _("Select Scanner: "));
    adw_combo_row_set_model(ADW_COMBO_ROW(m_DeviceSelectorList), G_LIST_MODEL(deviceList));
    gtk_list_box_append(GTK_LIST_BOX(m_DeviceSelectorRoot), m_DeviceSelectorList);
    m_DropdownSelectedSignalId = ConnectGtkSignalWithParamSpecs(
            this, &DeviceSelector::OnDeviceSelected, m_DeviceSelectorList, "notify::selected");

    auto networkScan = adw_switch_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(networkScan), _("Include Network Scanners"));
    adw_switch_row_set_active(ADW_SWITCH_ROW(networkScan), false);
    ConnectGtkSignalWithParamSpecs(this, &DeviceSelector::OnActivateNetwork, networkScan, "notify::active");
    gtk_list_box_append(GTK_LIST_BOX(m_DeviceSelectorRoot), networkScan);

    auto refreshButton = gtk_button_new_with_label(_("Refresh Scanner List"));
    gtk_widget_set_css_classes(GTK_WIDGET(refreshButton), buttonClasses);
    ConnectGtkSignal(this, &DeviceSelector::OnRefreshDevicesClicked, refreshButton, "clicked");
    gtk_list_box_append(GTK_LIST_BOX(m_DeviceSelectorRoot), refreshButton);

    m_Dispatcher.RegisterHandler(SelectDeviceCommand::Execute, m_State);
    m_Dispatcher.RegisterHandler(RefreshDeviceList::Execute, m_State);
    m_Dispatcher.RegisterHandler(ActivateNetworkScan::Execute, m_State);
}

Gorfector::DeviceSelector::~DeviceSelector()
{
    m_Dispatcher.UnregisterHandler<SelectDeviceCommand>();
    m_Dispatcher.UnregisterHandler<RefreshDeviceList>();
    m_Dispatcher.UnregisterHandler<ActivateNetworkScan>();

    gtk_widget_unparent(m_DeviceSelectorRoot);

    m_App->GetObserverManager()->RemoveObserver(m_Observer);
    delete m_Observer;
}

void Gorfector::DeviceSelector::OnRefreshDevicesClicked(GtkWidget *)
{
    m_Dispatcher.Dispatch(RefreshDeviceList());
}

void Gorfector::DeviceSelector::OnActivateNetwork(GtkWidget *widget)
{
    const bool value = adw_switch_row_get_active(ADW_SWITCH_ROW(widget));
    auto command = ActivateNetworkScan(value);
    m_Dispatcher.Dispatch(command);
}

void Gorfector::DeviceSelector::SelectDevice(const int deviceIndex)
{
    SaneDevice *device = nullptr;
    if (deviceIndex >= 0 && deviceIndex < static_cast<int>(m_State->GetDeviceList().size()))
    {
        device = m_State->GetDeviceList()[deviceIndex];
    }

    m_Dispatcher.Dispatch(
            SelectDeviceCommand(device == nullptr ? DeviceSelectorState::k_NullDeviceName : device->GetName()));
}

void Gorfector::DeviceSelector::OnDeviceSelected(GtkWidget *)
{
    const auto selectedIndex = static_cast<int>(adw_combo_row_get_selected(ADW_COMBO_ROW(m_DeviceSelectorList))) - 1;
    SelectDevice(selectedIndex);
}

void Gorfector::DeviceSelector::Update(const std::vector<uint64_t> &lastSeenVersion)
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

        devicesNames[dropDownItemIndex] = _("None");
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

            if (device->GetName() == m_State->GetSelectedDeviceName())
            {
                selectedItemIndex = deviceIndex + 1;
            }

            std::string deviceName = device->GetVendor();
            deviceName += " ";
            deviceName += device->GetModel();

            devicesNames[dropDownItemIndex] = deviceName;
            deviceNamesCStr[dropDownItemIndex] = devicesNames[dropDownItemIndex].c_str();

            deviceIndex++;
            dropDownItemIndex++;
        }

        deviceNamesCStr[dropDownItemIndex] = nullptr;
        try
        {
            if (m_DeviceSelectorList != nullptr)
            {
                auto *deviceGList = gtk_string_list_new(deviceNamesCStr.get());
                adw_combo_row_set_model(ADW_COMBO_ROW(m_DeviceSelectorList), G_LIST_MODEL(deviceGList));
                adw_combo_row_set_selected(ADW_COMBO_ROW(m_DeviceSelectorList), selectedItemIndex);
                gtk_widget_set_sensitive(m_DeviceSelectorList, true);
            }
        }
        catch (const SaneException &)
        {
            const auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
            ZooLib::ShowUserError(
                    ADW_APPLICATION_WINDOW(parentWindow),
                    std::vformat(_("Cannot open scanner {}."), std::make_format_args(deviceNamesCStr[0])));
        }
    }
    else
    {
        const auto parentWindow = gtk_widget_get_root(m_DeviceSelectorRoot);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(parentWindow), _("No scanner found."));

        if (m_DeviceSelectorList != nullptr)
        {
            const char *deviceListNames[] = {_("No Scanner Found"), nullptr};
            auto *deviceGList = gtk_string_list_new(deviceListNames);
            adw_combo_row_set_model(ADW_COMBO_ROW(m_DeviceSelectorList), G_LIST_MODEL(deviceGList));
            adw_combo_row_set_selected(ADW_COMBO_ROW(m_DeviceSelectorList), 0);
            gtk_widget_set_sensitive(m_DeviceSelectorList, false);
        }
    }

    g_signal_handler_unblock(m_DeviceSelectorList, m_DropdownSelectedSignalId);
}
