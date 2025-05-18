#include "ScanListPanel.hpp"

#include <format>

#include "ClearScanListCommand.hpp"
#include "Commands/DeleteScanItemCommand.hpp"
#include "Commands/LoadScanItemCommand.hpp"
#include "CreateScanListItemCommand.hpp"
#include "MoveScanListItemCommand.hpp"
#include "MultiScanProcess.hpp"
#include "PreviewPanel.hpp"

void Gorfector::ScanListPanel::BuildUI()
{
    m_RootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(m_RootWidget, 10);
    gtk_widget_set_margin_top(m_RootWidget, 0);
    gtk_widget_set_margin_start(m_RootWidget, 10);
    gtk_widget_set_margin_end(m_RootWidget, 10);
    gtk_widget_set_hexpand(m_RootWidget, TRUE);
    gtk_widget_set_vexpand(m_RootWidget, TRUE);

    const char *headingClasses[] = {"heading", nullptr};
    auto label = gtk_label_new(_("Scan List"));
    gtk_widget_set_margin_bottom(label, 10);
    gtk_widget_set_margin_top(label, 0);
    gtk_widget_set_margin_start(label, 0);
    gtk_widget_set_margin_end(label, 0);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_css_classes(label, headingClasses);
    gtk_box_append(GTK_BOX(m_RootWidget), label);

    auto scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_widget_set_size_request(scroller, 200, -1);
    gtk_box_append(GTK_BOX(m_RootWidget), scroller);

    auto viewport = gtk_viewport_new(gtk_adjustment_new(0, 0, 0, 0, 0, 0), gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), viewport);

    const char *scanListClasses[] = {"boxed-list", nullptr};
    m_ListBox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(m_ListBox), GTK_SELECTION_SINGLE);
    gtk_widget_set_margin_bottom(m_ListBox, 3);
    gtk_widget_set_margin_top(m_ListBox, 3);
    gtk_widget_set_margin_start(m_ListBox, 3);
    gtk_widget_set_margin_end(m_ListBox, 3);
    gtk_widget_set_hexpand(m_ListBox, TRUE);
    gtk_widget_set_vexpand(m_ListBox, FALSE);
    gtk_widget_set_valign(m_ListBox, GTK_ALIGN_START);
    gtk_widget_set_css_classes(m_ListBox, scanListClasses);
    ConnectGtkSignal(this, &ScanListPanel::OnItemSelected, m_ListBox, "row-selected");
    gtk_viewport_set_child(GTK_VIEWPORT(viewport), m_ListBox);

    auto upDownBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(upDownBox, 10);
    gtk_widget_set_halign(upDownBox, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(m_RootWidget), upDownBox);

    m_MoveItemUpButton = gtk_button_new();
    gtk_widget_set_css_classes(m_MoveItemUpButton, s_ButtonClasses);
    gtk_button_set_icon_name(GTK_BUTTON(m_MoveItemUpButton), "go-up-symbolic");
    gtk_widget_set_tooltip_text(m_MoveItemUpButton, _("Move Item Up"));
    ConnectGtkSignal(this, &ScanListPanel::OnMoveItemUpListClicked, m_MoveItemUpButton, "clicked");
    gtk_box_append(GTK_BOX(upDownBox), m_MoveItemUpButton);

    m_MoveItemDownButton = gtk_button_new();
    gtk_widget_set_css_classes(m_MoveItemDownButton, s_ButtonClasses);
    gtk_button_set_icon_name(GTK_BUTTON(m_MoveItemDownButton), "go-down-symbolic");
    gtk_widget_set_tooltip_text(m_MoveItemDownButton, _("Move Item Down"));
    ConnectGtkSignal(this, &ScanListPanel::OnMoveItemDownListClicked, m_MoveItemDownButton, "clicked");
    gtk_box_append(GTK_BOX(upDownBox), m_MoveItemDownButton);


    auto buttonBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_bottom(buttonBox, 20);
    gtk_widget_set_margin_top(buttonBox, 20);
    gtk_widget_set_margin_start(buttonBox, 0);
    gtk_widget_set_margin_end(buttonBox, 0);
    gtk_box_append(GTK_BOX(m_RootWidget), buttonBox);

    auto menuModel = g_menu_new();
    g_menu_append(menuModel, _("Add Scan Area"), "app.add-scan-area");
    g_menu_append(menuModel, _("Add All Params"), "app.add-all-params");
    m_App->BindMethodToAction<ScanListPanel>("add-scan-area", &ScanListPanel::SetAddScanArea, this);
    m_App->BindMethodToAction<ScanListPanel>("add-all-params", &ScanListPanel::SetAddAllParams, this);

    m_AddToScanListButton = adw_split_button_new();
    adw_split_button_set_menu_model(ADW_SPLIT_BUTTON(m_AddToScanListButton), G_MENU_MODEL(menuModel));
    ConnectGtkSignal(this, &ScanListPanel::OnAddToScanListClicked, m_AddToScanListButton, "clicked");
    gtk_box_append(GTK_BOX(buttonBox), m_AddToScanListButton);

    m_ClearScanListButton = gtk_button_new_with_label(_("Clear List"));
    gtk_widget_set_css_classes(m_ClearScanListButton, s_ButtonClasses);
    ConnectGtkSignal(this, &ScanListPanel::OnClearScanListClicked, m_ClearScanListButton, "clicked");
    gtk_box_append(GTK_BOX(buttonBox), m_ClearScanListButton);

    auto horizontalSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_bottom(horizontalSeparator, 10);
    gtk_widget_set_margin_top(horizontalSeparator, 10);
    gtk_widget_set_margin_start(horizontalSeparator, 20);
    gtk_widget_set_margin_end(horizontalSeparator, 20);
    gtk_widget_set_hexpand(horizontalSeparator, TRUE);
    gtk_widget_set_vexpand(horizontalSeparator, FALSE);
    gtk_widget_set_valign(horizontalSeparator, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(buttonBox), horizontalSeparator);

    m_ScanListButton = gtk_button_new_with_label(_("Scan All"));
    gtk_widget_set_css_classes(m_ScanListButton, s_ButtonClasses);
    ConnectGtkSignal(this, &ScanListPanel::OnScanClicked, m_ScanListButton, "clicked");
    gtk_box_append(GTK_BOX(buttonBox), m_ScanListButton);

    m_CancelListButton = gtk_button_new_with_label(_("Cancel Scans"));
    gtk_widget_set_css_classes(m_CancelListButton, s_ButtonClasses);
    ConnectGtkSignal(this, &ScanListPanel::OnCancelClicked, m_CancelListButton, "clicked");
    gtk_box_append(GTK_BOX(buttonBox), m_CancelListButton);
}

void Gorfector::ScanListPanel::OnItemSelected(GtkListBox *listBox, GtkListBoxRow *row)
{
    if (m_BlockOnItemSelected)
    {
        return;
    }

    int rowId = -1;
    if (row != nullptr)
    {
        rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
    }
    if (rowId != m_PanelState->GetSelectedIndex())
    {
        auto updater = ScanListState::Updater(m_PanelState);
        updater.SetSelectedIndex(rowId);
    }
}

void Gorfector::ScanListPanel::OnMoveItemUpListClicked(GtkWidget *widget)
{
    auto selectedRow = gtk_list_box_get_selected_row(GTK_LIST_BOX(m_ListBox));
    if (selectedRow == nullptr)
    {
        return;
    }

    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(selectedRow));
    m_Dispatcher.Dispatch(MoveScanListItemCommand(rowId, -1));
}

void Gorfector::ScanListPanel::OnMoveItemDownListClicked(GtkWidget *widget)
{
    auto selectedRow = gtk_list_box_get_selected_row(GTK_LIST_BOX(m_ListBox));
    if (selectedRow == nullptr)
    {
        return;
    }

    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(selectedRow));
    m_Dispatcher.Dispatch(MoveScanListItemCommand(rowId, 1));
}

void Gorfector::ScanListPanel::SetAddScanArea(GSimpleAction *action, GVariant *parameter)
{
    m_Dispatcher.Dispatch(SetAddToScanListAddsAllParamsCommand(false));
}

void Gorfector::ScanListPanel::SetAddAllParams(GSimpleAction *action, GVariant *parameter)
{
    m_Dispatcher.Dispatch(SetAddToScanListAddsAllParamsCommand(true));
}

void Gorfector::ScanListPanel::OnAddToScanListClicked(GtkWidget *widget)
{
    auto scanOptions = m_App->GetDeviceOptions();
    auto outputOptions = m_App->GetOutputOptions();

    if (scanOptions == nullptr || outputOptions == nullptr)
    {
        return;
    }

    auto destination = outputOptions->GetOutputDestination();
    if (destination == OutputOptionsState::OutputDestination::e_File)
    {
        if (!m_App->CheckFileOutputOptions(outputOptions))
        {
            return;
        }
    }

    if (m_PanelState->GetAddToScanListButtonAddsAllParams())
    {
        m_Dispatcher.Dispatch(CreateScanListItemCommand(scanOptions, outputOptions));
    }
    else
    {
        m_Dispatcher.Dispatch(CreateScanListItemCommand(scanOptions));
    }
}

void Gorfector::ScanListPanel::OnClearScanListClicked(GtkWidget *widget)
{
    auto alert = adw_alert_dialog_new(_("Clear Scan List"), nullptr);
    adw_alert_dialog_format_body(ADW_ALERT_DIALOG(alert), _("Are you sure you want to clear the scan list?"));
    adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(alert), "cancel", _("Cancel"), "delete", _("Clear"), NULL);
    adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG(alert), "delete", ADW_RESPONSE_DESTRUCTIVE);
    adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(alert), "cancel");
    adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(alert), "cancel");
    ZooLib::ConnectGtkSignal(this, &ScanListPanel::OnDeleteListAlertResponse, alert, "response");
    adw_dialog_present(alert, GTK_WIDGET(m_App->GetMainWindow()));
}

void Gorfector::ScanListPanel::OnDeleteListAlertResponse(AdwAlertDialog *alert, gchar *response)
{
    if (strcmp(response, "delete") == 0)
    {
        m_Dispatcher.Dispatch(ClearScanListCommand());
    }
}

void Gorfector::ScanListPanel::OnScanClicked(GtkWidget *widget)
{
    auto *finishCallback = new std::function<void()>([this]() { this->m_ScanProcess = nullptr; });

    auto appState = m_App->GetAppState();
    auto deviceSelectorState = m_App->GetDeviceSelectorState();
    auto currentDeviceName = appState->GetCurrentDeviceName();
    auto currentDevice = deviceSelectorState->GetDeviceByName(currentDeviceName);

    m_ScanProcess = new MultiScanProcess(
            currentDevice, m_PanelState, m_App->GetPreviewPanel()->GetState(), appState, m_App->GetDeviceOptions(),
            m_App->GetOutputOptions(), GTK_WIDGET(m_App->GetMainWindow()), finishCallback);

    if (!m_ScanProcess->Start())
    {
        // finishCallback is deleted in the destructor of ScanProcess
        delete m_ScanProcess;
        m_ScanProcess = nullptr;
    }
}

void Gorfector::ScanListPanel::OnCancelClicked(GtkWidget *widget)
{
    if (m_ScanProcess != nullptr)
    {
        m_ScanProcess->Cancel();
    }
}

namespace Gorfector
{
    GtkWidget *CreateScanListItem(gpointer item, gpointer userData)
    {
        auto panel = static_cast<ScanListPanel *>(userData);
        return panel->CreateScanListItem(gtk_string_object_get_string(GTK_STRING_OBJECT(item)));
    }
}

GtkWidget *Gorfector::ScanListPanel::CreateScanListItem(const char *itemName)
{
    if (itemName == nullptr)
        return nullptr;

    const char *buttonClasses[] = {"flat", nullptr};

    auto itemBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    auto labelItem = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(labelItem), itemName);
    gtk_widget_set_margin_top(labelItem, 10);
    gtk_widget_set_margin_bottom(labelItem, 10);
    gtk_widget_set_margin_start(labelItem, 10);
    gtk_widget_set_margin_end(labelItem, 10);
    gtk_widget_set_hexpand(labelItem, true);
    gtk_widget_set_halign(labelItem, GTK_ALIGN_START);
    gtk_widget_set_vexpand(labelItem, true);
    gtk_box_append(GTK_BOX(itemBox), labelItem);

    auto loadButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(loadButton), "playlist-symbolic");
    gtk_widget_set_name(loadButton, "apply-button");
    gtk_widget_set_tooltip_text(loadButton, _("Load"));
    gtk_widget_set_halign(loadButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(loadButton), buttonClasses);
    gtk_widget_set_margin_top(loadButton, 3);
    gtk_widget_set_margin_bottom(loadButton, 3);
    gtk_widget_set_margin_start(loadButton, 3);
    gtk_widget_set_margin_end(loadButton, 3);
    gtk_widget_set_size_request(loadButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), loadButton);
    ZooLib::ConnectGtkSignal(this, &ScanListPanel::OnLoadButtonPressed, loadButton, "clicked");

    auto deleteButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(deleteButton), "list-remove-symbolic");
    gtk_widget_set_tooltip_text(deleteButton, _("Delete"));
    gtk_widget_set_halign(deleteButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(deleteButton), buttonClasses);
    gtk_widget_set_margin_top(deleteButton, 3);
    gtk_widget_set_margin_bottom(deleteButton, 3);
    gtk_widget_set_margin_start(deleteButton, 3);
    gtk_widget_set_margin_end(deleteButton, 3);
    gtk_widget_set_size_request(deleteButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), deleteButton);
    ZooLib::ConnectGtkSignal(this, &ScanListPanel::OnDeleteItemButtonPressed, deleteButton, "clicked");

    return itemBox;
}

void Gorfector::ScanListPanel::OnLoadButtonPressed(GtkButton *button)
{
    auto listBoxRow = ZooLib::GetParentOfType(GTK_WIDGET(button), GTK_TYPE_LIST_BOX_ROW);
    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(listBoxRow));
    auto scannerSettings = m_PanelState->GetScannerSettings(rowId);
    auto outputSettings = m_PanelState->GetOutputSettings(rowId);
    auto scanAreaSettings = m_PanelState->GetScanAreaSettings(rowId);
    m_Dispatcher.Dispatch(LoadScanItemCommand(scannerSettings, outputSettings, scanAreaSettings));
}

void Gorfector::ScanListPanel::OnDeleteItemButtonPressed(GtkButton *button)
{
    auto listBoxRow = ZooLib::GetParentOfType(GTK_WIDGET(button), GTK_TYPE_LIST_BOX_ROW);
    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(listBoxRow));
    auto itemId = m_PanelState->GetScanItemId(rowId);

    auto alert = adw_alert_dialog_new(_("Delete Scan"), nullptr);
    adw_alert_dialog_format_body(
            ADW_ALERT_DIALOG(alert), _("Are you sure you want to delete the scan item #%03d ?"), itemId);
    adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(alert), "cancel", _("Cancel"), "delete", _("Delete"), NULL);
    adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG(alert), "delete", ADW_RESPONSE_DESTRUCTIVE);
    adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(alert), "cancel");
    adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(alert), "cancel");
    g_object_set_data(G_OBJECT(alert), "ScanItemIndex", GINT_TO_POINTER(rowId));
    ZooLib::ConnectGtkSignal(this, &ScanListPanel::OnDeleteItemAlertResponse, alert, "response");
    adw_dialog_present(alert, GTK_WIDGET(m_App->GetMainWindow()));
}

void Gorfector::ScanListPanel::OnDeleteItemAlertResponse(AdwAlertDialog *alert, gchar *response)
{
    if (strcmp(response, "delete") == 0)
    {
        auto scanItemIndex = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(alert), "ScanItemIndex"));
        m_Dispatcher.Dispatch(DeleteScanItemCommand(scanItemIndex));
    }
}

void Gorfector::ScanListPanel::Update(const std::vector<uint64_t> &lastSeenVersions)
{
    if (m_PanelState == nullptr)
        return;

    bool forceUpdate = lastSeenVersions[0] == 0;

    auto changeset = m_PanelState->GetAggregatedChangeset(lastSeenVersions[0]);
    if (!forceUpdate && (changeset == nullptr || !changeset->HasAnyChange()))
    {
        return;
    }

    if (forceUpdate || changeset->IsChanged(ScanListStateChangeset::TypeFlag::ListContent))
    {
        m_BlockOnItemSelected = true;

        m_Dispatcher.UnregisterHandler<LoadScanItemCommand>();
        m_Dispatcher.RegisterHandler(
                LoadScanItemCommand::Execute, m_App->GetDeviceOptions(), m_App->GetOutputOptions());

        m_ScanListItemNames.clear();

        auto scanCount = m_PanelState->GetScanListSize();
        const char **names = new const char *[scanCount + 1];
        std::string units{};
        int itemId{};
        double tlx{}, tly{}, brx{}, bry{};
        bool isScanAreaItem{};
        m_ScanListItemNames.clear();
        for (size_t i = 0; i < scanCount; ++i)
        {
            m_PanelState->GetScanItemInfos(i, itemId, units, tlx, tly, brx, bry, isScanAreaItem);
            auto type = isScanAreaItem ? "◰" : "▤";
            auto labelText = std::format(
                    "{} <b>#{:03}</b> <small>({:.2f}; {:.2f}) - ({:.2f}; {:.2f}) {}</small>", type, itemId, tlx, tly,
                    brx, bry, units);
            m_ScanListItemNames.push_back(labelText);
        }
        for (size_t i = 0; i < scanCount; ++i)
        {
            names[i] = m_ScanListItemNames[i].c_str();
        }
        names[scanCount] = nullptr;

        auto scanListModel = gtk_string_list_new(names);
        delete[] names;

        gtk_list_box_bind_model(
                GTK_LIST_BOX(m_ListBox), G_LIST_MODEL(scanListModel), &Gorfector::CreateScanListItem, this, nullptr);

        if (scanCount == 0)
        {
            gtk_widget_set_sensitive(m_ScanListButton, false);
            gtk_widget_set_sensitive(m_ClearScanListButton, false);
            gtk_widget_set_sensitive(m_CancelListButton, false);
        }
        else
        {
            gtk_widget_set_sensitive(m_ScanListButton, true);
            gtk_widget_set_sensitive(m_ClearScanListButton, true);
            gtk_widget_set_sensitive(m_CancelListButton, true);
        }

        m_BlockOnItemSelected = false;
    }

    if (forceUpdate || changeset->IsChanged(ScanListStateChangeset::TypeFlag::ButtonAction))
    {
        if (m_PanelState->GetAddToScanListButtonAddsAllParams())
        {
            adw_split_button_set_label(ADW_SPLIT_BUTTON(m_AddToScanListButton), _("Add All Params"));
        }
        else
        {
            adw_split_button_set_label(ADW_SPLIT_BUTTON(m_AddToScanListButton), _("Add Scan Area"));
        }
    }

    if (forceUpdate || changeset->IsChanged(ScanListStateChangeset::TypeFlag::SelectedItem))
    {
        GtkListBoxRow *selectedRow = nullptr;
        auto selectedIndex = m_PanelState->GetSelectedIndex();
        if (selectedIndex >= 0)
        {
            selectedRow = gtk_list_box_get_row_at_index(GTK_LIST_BOX(m_ListBox), selectedIndex);
            gtk_widget_set_sensitive(m_MoveItemUpButton, true);
            gtk_widget_set_sensitive(m_MoveItemDownButton, true);
        }
        else
        {
            gtk_widget_set_sensitive(m_MoveItemUpButton, false);
            gtk_widget_set_sensitive(m_MoveItemDownButton, false);
        }

        gtk_list_box_select_row(GTK_LIST_BOX(m_ListBox), selectedRow);
    }
}
