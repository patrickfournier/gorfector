#include "PresetPanel.hpp"

#include <format>

#include "Commands/ApplyPresetCommand.hpp"
#include "ZooLib/GtkUtils.hpp"

enum class ItemButtons
{
    e_Apply = 1,
    e_View,
    e_Rename,
    e_Delete
};

void ZooScan::PresetPanel::BuildUI()
{
    const char *buttonClasses[] = {"flat", nullptr};
    const char *rootClasses[] = {"card", "no-bottom", nullptr};
    const char *listBoxClasses[] = {"boxed-list", nullptr};

    m_RootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(m_RootWidget, -10);
    gtk_widget_set_css_classes(m_RootWidget, rootClasses);

    m_Expander = gtk_expander_new("");
    gtk_widget_set_margin_top(m_Expander, 10);
    gtk_widget_set_margin_bottom(m_Expander, 10);
    gtk_widget_set_margin_start(m_Expander, 10);
    gtk_widget_set_margin_end(m_Expander, 10);
    gtk_widget_remove_css_class(m_Expander, "card");
    gtk_expander_set_expanded(GTK_EXPANDER(m_Expander), m_PresetPanelState->IsExpanded());
    gtk_box_append(GTK_BOX(m_RootWidget), m_Expander);
    ZooLib::ConnectGtkSignalWithParamSpecs(this, &PresetPanel::OnExpanderExpanded, m_Expander, "notify::expanded");

    auto headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(headerBox, true);
    gtk_expander_set_label_widget(GTK_EXPANDER(m_Expander), headerBox);

    const char *labelClasses[] = {"heading", "flat", nullptr};
    auto labelItem = gtk_label_new(_("Presets"));
    gtk_widget_set_css_classes(labelItem, labelClasses);
    gtk_widget_remove_css_class(labelItem, "card");
    gtk_widget_remove_css_class(labelItem, "no-bottom");
    gtk_box_append(GTK_BOX(headerBox), labelItem);

    m_CreatePresetButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(m_CreatePresetButton), "list-add-symbolic");
    gtk_widget_set_tooltip_text(m_CreatePresetButton, _("New preset"));
    gtk_widget_set_css_classes(GTK_WIDGET(m_CreatePresetButton), buttonClasses);
    gtk_widget_set_halign(m_CreatePresetButton, GTK_ALIGN_END);
    gtk_widget_set_margin_top(labelItem, 3);
    gtk_widget_set_margin_bottom(labelItem, 3);
    gtk_widget_set_margin_start(labelItem, 3);
    gtk_widget_set_margin_end(labelItem, 3);
    gtk_widget_set_size_request(m_CreatePresetButton, 32, 32);
    gtk_widget_set_hexpand(m_CreatePresetButton, true);
    gtk_box_append(GTK_BOX(headerBox), m_CreatePresetButton);
    g_signal_connect(m_CreatePresetButton, "clicked", G_CALLBACK(ZooScan::ShowCreatePresetDialog), this);

    auto scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_widget_set_size_request(scroller, -1, 150);
    gtk_expander_set_child(GTK_EXPANDER(m_Expander), scroller);

    m_ListBox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(m_ListBox), GTK_SELECTION_NONE);
    gtk_widget_set_css_classes(GTK_WIDGET(m_ListBox), listBoxClasses);
    gtk_widget_set_margin_bottom(m_ListBox, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), m_ListBox);
}

void ZooScan::PresetPanel::OnApplyPresetButtonPressed(GtkButton *button)
{
    auto listBoxRow = gtk_widget_get_parent(GTK_WIDGET(button));
    while (listBoxRow != nullptr && !GTK_IS_LIST_BOX_ROW(listBoxRow))
    {
        listBoxRow = gtk_widget_get_parent(listBoxRow);
    }
    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(listBoxRow));
    auto presetName = m_DisplayedPresetNames[rowId];
    auto preset = m_PresetPanelState->GetPreset(presetName);
    m_Dispatcher.Dispatch(ApplyPresetCommand(preset));
}

void ZooScan::PresetPanel::OnDeletePresetButtonPressed(GtkButton *button)
{
    auto listBoxRow = gtk_widget_get_parent(GTK_WIDGET(button));
    while (listBoxRow != nullptr && !GTK_IS_LIST_BOX_ROW(listBoxRow))
    {
        listBoxRow = gtk_widget_get_parent(listBoxRow);
    }

    auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(listBoxRow));
    auto presetName = m_DisplayedPresetNames[rowId];

    auto alert = adw_alert_dialog_new(_("Delete Preset"), nullptr);
    adw_alert_dialog_format_body(
            ADW_ALERT_DIALOG(alert), _("Are you sure you want to delete the preset \"%s\"?"), presetName.c_str());
    adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(alert), "cancel", _("Cancel"), "delete", _("Delete"), NULL);
    adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG(alert), "delete", ADW_RESPONSE_DESTRUCTIVE);
    adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(alert), "cancel");
    adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(alert), "cancel");
    g_object_set_data(G_OBJECT(alert), "PresetName", strdup(presetName.c_str()));
    ZooLib::ConnectGtkSignal(this, &PresetPanel::OnDeleteAlertResponse, alert, "response");
    adw_dialog_present(alert, GTK_WIDGET(m_App->GetMainWindow()));
}

void ZooScan::PresetPanel::OnDeleteAlertResponse(AdwAlertDialog *alert, gchar *response)
{
    if (strcmp(response, "delete") == 0)
    {
        auto presetName = static_cast<char *>(g_object_get_data(G_OBJECT(alert), "PresetName"));
        m_Dispatcher.Dispatch(DeletePresetCommand(presetName));
        free(presetName);
    }
}

GtkWidget *ZooScan::CreatePresetListItem(gpointer item, gpointer userData)
{
    auto presetPanel = static_cast<PresetPanel *>(userData);
    return presetPanel->CreatePresetListItem(gtk_string_object_get_string(GTK_STRING_OBJECT(item)));
}

GtkWidget *ZooScan::PresetPanel::CreatePresetListItem(const char *itemName)
{
    if (itemName == nullptr)
        return nullptr;

    const char *buttonClasses[] = {"flat", nullptr};

    auto itemBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    auto labelItem = gtk_label_new(itemName);
    gtk_widget_set_margin_top(labelItem, 10);
    gtk_widget_set_margin_bottom(labelItem, 10);
    gtk_widget_set_margin_start(labelItem, 10);
    gtk_widget_set_margin_end(labelItem, 10);
    gtk_widget_set_hexpand(labelItem, true);
    gtk_widget_set_halign(labelItem, GTK_ALIGN_START);
    gtk_widget_set_vexpand(labelItem, true);
    gtk_box_append(GTK_BOX(itemBox), labelItem);

    auto applyButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(applyButton), "org.gnome.Lollypop-play-queue-symbolic");
    gtk_widget_set_name(applyButton, "apply-button");
    gtk_widget_set_tooltip_text(applyButton, _("Apply this preset"));
    gtk_widget_set_halign(applyButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(applyButton), buttonClasses);
    gtk_widget_set_margin_top(labelItem, 3);
    gtk_widget_set_margin_bottom(labelItem, 3);
    gtk_widget_set_margin_start(labelItem, 3);
    gtk_widget_set_margin_end(labelItem, 3);
    gtk_widget_set_size_request(applyButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), applyButton);
    g_object_set_data(G_OBJECT(applyButton), "ButtonId", GINT_TO_POINTER(ItemButtons::e_Apply));
    ZooLib::ConnectGtkSignal(this, &PresetPanel::OnApplyPresetButtonPressed, applyButton, "clicked");

    auto viewButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(viewButton), "docviewer-app-symbolic");
    gtk_widget_set_tooltip_text(viewButton, _("Inspect this preset"));
    gtk_widget_set_halign(viewButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(viewButton), buttonClasses);
    gtk_widget_set_margin_top(viewButton, 3);
    gtk_widget_set_margin_bottom(viewButton, 3);
    gtk_widget_set_margin_start(viewButton, 3);
    gtk_widget_set_margin_end(viewButton, 3);
    gtk_widget_set_size_request(viewButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), viewButton);
    g_object_set_data(G_OBJECT(viewButton), "ButtonId", GINT_TO_POINTER(ItemButtons::e_View));
    g_signal_connect(viewButton, "clicked", G_CALLBACK(ZooScan::ShowViewPresetDialog), this);

    auto renameButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(renameButton), "document-edit-symbolic");
    gtk_widget_set_tooltip_text(renameButton, _("Rename this preset"));
    gtk_widget_set_halign(renameButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(renameButton), buttonClasses);
    gtk_widget_set_margin_top(renameButton, 3);
    gtk_widget_set_margin_bottom(renameButton, 3);
    gtk_widget_set_margin_start(renameButton, 3);
    gtk_widget_set_margin_end(renameButton, 3);
    gtk_widget_set_size_request(renameButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), renameButton);
    g_object_set_data(G_OBJECT(renameButton), "ButtonId", GINT_TO_POINTER(ItemButtons::e_Rename));
    g_signal_connect(renameButton, "clicked", G_CALLBACK(ZooScan::ShowRenamePresetDialog), this);

    auto deleteButton = gtk_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(deleteButton), "list-remove-symbolic");
    gtk_widget_set_tooltip_text(deleteButton, _("Delete this preset"));
    gtk_widget_set_halign(deleteButton, GTK_ALIGN_END);
    gtk_widget_set_css_classes(GTK_WIDGET(deleteButton), buttonClasses);
    gtk_widget_set_margin_top(deleteButton, 3);
    gtk_widget_set_margin_bottom(deleteButton, 3);
    gtk_widget_set_margin_start(deleteButton, 3);
    gtk_widget_set_margin_end(deleteButton, 3);
    gtk_widget_set_size_request(deleteButton, 32, 32);
    gtk_box_append(GTK_BOX(itemBox), deleteButton);
    g_object_set_data(G_OBJECT(deleteButton), "ButtonId", GINT_TO_POINTER(ItemButtons::e_Delete));
    ZooLib::ConnectGtkSignal(this, &PresetPanel::OnDeletePresetButtonPressed, deleteButton, "clicked");

    return itemBox;
}
