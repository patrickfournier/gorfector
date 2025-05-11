#include <adwaita.h>

#include <nlohmann/json.hpp>
#include "PresetPanel.hpp"
#include "PresetPanelDialogs.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/GtkUtils.hpp"
#include "ZooLib/StringUtils.hpp"

namespace Gorfector
{
    static void OnPresetNameUpdated(GtkEntryBuffer *entryBuffer, GParamSpec *paramSpec, gpointer userData)
    {
        auto dialog = ADW_DIALOG(userData);
        auto presetPanel = static_cast<PresetPanel *>(g_object_get_data(G_OBJECT(dialog), "PresetPanel"));
        auto text = gtk_entry_buffer_get_text(entryBuffer);
        auto entryHintLabel = ZooLib::FindWidgetByName(GTK_WIDGET(dialog), "preset-name-hint");
        auto createButton = ZooLib::FindWidgetByName(GTK_WIDGET(dialog), "apply-button");

        std::string presetName(text);
        ZooLib::Trim(presetName);

        if (presetName.empty())
        {
            gtk_label_set_text(GTK_LABEL(entryHintLabel), _(s_EmptyNameHint));
            gtk_widget_set_sensitive(createButton, false);
        }
        else if (!presetPanel->IsUniquePresetName(presetName))
        {
            gtk_label_set_text(GTK_LABEL(entryHintLabel), _(s_NameConflictHint));
            gtk_widget_set_sensitive(createButton, false);
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(entryHintLabel), "");
            gtk_widget_set_sensitive(createButton, true);
        }
    }

    void OnCreatePreset(GtkWidget *widget, gpointer userData)
    {
        auto presetPanel = static_cast<PresetPanel *>(userData);
        if (presetPanel == nullptr)
        {
            return;
        }

        auto dialog = ZooLib::GetParentOfType(widget, ADW_TYPE_DIALOG);
        if (dialog == nullptr)
        {
            g_warn_message(App::k_ApplicationName, __FILE__, __LINE__, "", "Dialog not found");
            return;
        }

        auto entry = ZooLib::FindWidgetByName(dialog, "preset-name");
        if (entry == nullptr)
        {
            g_warn_message(App::k_ApplicationName, __FILE__, __LINE__, "", "Preset name entry not found");
            return;
        }

        auto entryTextBuffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
        auto entryText = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entryTextBuffer));
        auto presetId = std::string(entryText);

        auto scannerSettingsCB = ZooLib::FindWidgetByName(dialog, "scanner-settings");
        if (scannerSettingsCB == nullptr)
        {
            g_warn_message(App::k_ApplicationName, __FILE__, __LINE__, "", "Scanner settings checkbox not found");
            return;
        }
        auto scanAreaCB = ZooLib::FindWidgetByName(dialog, "scan-area");
        if (scanAreaCB == nullptr)
        {
            g_warn_message(App::k_ApplicationName, __FILE__, __LINE__, "", "Scan area checkbox not found");
            return;
        }
        auto outputSettingsCB = ZooLib::FindWidgetByName(dialog, "output-settings");
        if (outputSettingsCB == nullptr)
        {
            g_warn_message(App::k_ApplicationName, __FILE__, __LINE__, "", "Output settings checkbox not found");
            return;
        }

        auto saveScannerSettings = gtk_check_button_get_active(GTK_CHECK_BUTTON(scannerSettingsCB));
        auto saveScanArea = gtk_check_button_get_active(GTK_CHECK_BUTTON(scanAreaCB));
        auto saveOutputSettings = gtk_check_button_get_active(GTK_CHECK_BUTTON(outputSettingsCB));

        presetPanel->CreatePreset(presetId, saveScannerSettings, saveScanArea, saveOutputSettings);

        adw_dialog_close(ADW_DIALOG(dialog));
    }

    void ShowCreatePresetDialog(GtkWidget *widget, gpointer userData)
    {
        auto presetPanel = static_cast<PresetPanel *>(userData);
        if (presetPanel == nullptr)
        {
            return;
        }

        auto dialog = adw_dialog_new();
        g_object_set_data(G_OBJECT(dialog), "PresetPanel", userData);
        adw_dialog_set_title(dialog, _("Create a new preset"));

        auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        adw_dialog_set_child(dialog, box);

        auto header = adw_header_bar_new();
        gtk_box_append(GTK_BOX(box), header);

        auto hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_margin_bottom(hbox1, 10);
        gtk_widget_set_margin_top(hbox1, 10);
        gtk_widget_set_margin_start(hbox1, 10);
        gtk_widget_set_margin_end(hbox1, 10);
        gtk_box_append(GTK_BOX(box), hbox1);

        auto label = gtk_label_new(_("Enter a name for the new preset:"));
        gtk_widget_set_margin_end(label, 10);
        gtk_box_append(GTK_BOX(hbox1), label);

        auto entry = gtk_entry_new();
        gtk_widget_set_name(entry, "preset-name");
        gtk_box_append(GTK_BOX(hbox1), entry);
        auto entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
        g_signal_connect(entryBuffer, "notify::text", G_CALLBACK(OnPresetNameUpdated), dialog);

        auto hbox2 = gtk_flow_box_new();
        gtk_widget_set_margin_bottom(hbox2, 10);
        gtk_widget_set_margin_start(hbox2, 10);
        gtk_widget_set_margin_end(hbox2, 10);
        gtk_box_append(GTK_BOX(box), hbox2);

        auto incScanner = gtk_check_button_new_with_label(_("Scanners Settings"));
        gtk_widget_set_name(incScanner, "scanner-settings");
        gtk_check_button_set_active(GTK_CHECK_BUTTON(incScanner), true);
        gtk_flow_box_append(GTK_FLOW_BOX(hbox2), incScanner);

        auto incScanArea = gtk_check_button_new_with_label(_("Scan Area"));
        gtk_widget_set_name(incScanArea, "scan-area");
        gtk_check_button_set_active(GTK_CHECK_BUTTON(incScanArea), false);
        gtk_flow_box_append(GTK_FLOW_BOX(hbox2), incScanArea);

        auto incOutput = gtk_check_button_new_with_label(_("Output Settings"));
        gtk_widget_set_name(incOutput, "output-settings");
        gtk_check_button_set_active(GTK_CHECK_BUTTON(incOutput), true);
        gtk_flow_box_append(GTK_FLOW_BOX(hbox2), incOutput);

        auto bottomBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_margin_bottom(bottomBox, 10);
        gtk_widget_set_margin_start(bottomBox, 10);
        gtk_widget_set_margin_end(bottomBox, 10);
        gtk_box_append(GTK_BOX(box), bottomBox);

        auto entryHint = gtk_label_new(_(s_EmptyNameHint));
        gtk_widget_set_name(entryHint, "preset-name-hint");
        gtk_widget_set_css_classes(entryHint, s_EntryHintClasses);
        gtk_widget_set_halign(entryHint, GTK_ALIGN_START);
        gtk_widget_set_hexpand(entryHint, true);
        gtk_box_append(GTK_BOX(bottomBox), entryHint);

        auto createButton = gtk_button_new_with_label(_("Create"));
        gtk_widget_set_name(createButton, "apply-button");
        gtk_widget_set_css_classes(createButton, s_ButtonClasses);
        gtk_widget_set_halign(createButton, GTK_ALIGN_END);
        gtk_widget_set_sensitive(createButton, false);
        gtk_box_append(GTK_BOX(bottomBox), createButton);
        g_signal_connect(createButton, "clicked", G_CALLBACK(OnCreatePreset), presetPanel);

        adw_dialog_present(dialog, GTK_WIDGET(presetPanel->GetMainWindow()));
    }
}
