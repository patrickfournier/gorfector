#include "PresetPanelDialogs.hpp"

#include <adwaita.h>

#include "PresetPanel.hpp"

namespace Gorfector
{
    void ShowViewPresetDialog(GtkWidget *widget, gpointer userData)
    {
        auto listBoxRow = ZooLib::GetParentOfType(widget, GTK_TYPE_LIST_BOX_ROW);
        auto rowId = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(listBoxRow));

        auto presetPanel = static_cast<PresetPanel *>(userData);
        if (presetPanel == nullptr)
        {
            return;
        }

        auto dialog = adw_dialog_new();
        adw_dialog_set_title(dialog, _("Inspect Preset"));
        adw_dialog_set_follows_content_size(dialog, true);

        auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        adw_dialog_set_child(dialog, box);

        auto header = adw_header_bar_new();
        gtk_box_append(GTK_BOX(box), header);

        auto hbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_margin_bottom(hbox1, 15);
        gtk_widget_set_margin_top(hbox1, 15);
        gtk_widget_set_margin_start(hbox1, 15);
        gtk_widget_set_margin_end(hbox1, 15);
        gtk_box_append(GTK_BOX(box), hbox1);

        auto preset = presetPanel->GetPreset(rowId);

        auto label = gtk_label_new((*preset)[PresetPanelState::k_PresetNameKey].get<std::string>().c_str());
        gtk_widget_set_css_classes(label, s_TitleClasses);
        gtk_widget_set_hexpand(label, true);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(hbox1), label);

        auto scroller = gtk_scrolled_window_new();
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scroller), 600);
        gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
        gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), true);
        gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroller), true);
        gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroller), true);
        gtk_box_append(GTK_BOX(hbox1), scroller);

        auto textView = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), false);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textView), false);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_NONE);
        gtk_text_view_set_monospace(GTK_TEXT_VIEW(textView), true);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), textView);

        auto textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
        auto presetText = preset->dump(4);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(textBuffer), presetText.c_str(), -1);

        adw_dialog_present(dialog, GTK_WIDGET(presetPanel->GetMainWindow()));
    }
}
