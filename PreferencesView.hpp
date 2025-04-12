#pragma once

#include "ZooLib/View.hpp"

#include <Writers/TiffWriterStateComponent.hpp>

namespace ZooScan
{

    class PreferencesView : public ZooLib::View
    {
        GtkWidget *m_RootElement{};

        static void BuildFileSettingsBox(GtkWidget *parentBox)
        {
            auto frame = gtk_frame_new("TIFF Settings");
            gtk_box_append(GTK_BOX(parentBox), frame);
            auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_frame_set_child(GTK_FRAME(frame), box);
            gtk_widget_add_css_class(frame, "settings-group");
            gtk_widget_set_tooltip_text(frame, "TIFF Settings");
            gtk_widget_set_visible(frame, true);
            gtk_widget_set_hexpand(frame, true);
            gtk_widget_set_vexpand(frame, true);

            auto lineBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_append(GTK_BOX(box), lineBox);
            auto label = gtk_label_new("Compression Algorithm: ");
            gtk_box_append(GTK_BOX(lineBox), label);
            auto dropDown =
                    gtk_drop_down_new_from_strings(TiffWriterStateComponent::GetCompressionAlgorithmNames().data());
            gtk_widget_set_tooltip_text(dropDown, "Compression Algorithm");
            gtk_box_append(GTK_BOX(lineBox), dropDown);

            lineBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_append(GTK_BOX(box), lineBox);
            label = gtk_label_new("Deflate Compression Level: ");
            gtk_box_append(GTK_BOX(lineBox), label);
            auto spinButton = gtk_spin_button_new_with_range(1, 9, 1);
            gtk_box_append(GTK_BOX(lineBox), spinButton);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinButton), 1);
            gtk_widget_set_tooltip_text(spinButton, "Deflate Compression Level");

            lineBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_append(GTK_BOX(box), lineBox);
            label = gtk_label_new("JPEG Quality: ");
            gtk_box_append(GTK_BOX(lineBox), label);
            spinButton = gtk_spin_button_new_with_range(1, 100, 1);
            gtk_box_append(GTK_BOX(lineBox), spinButton);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinButton), 75);
            gtk_widget_set_tooltip_text(spinButton, "JPEG Quality");

            frame = gtk_frame_new("PNG Settings");
            gtk_box_append(GTK_BOX(parentBox), frame);
            box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_frame_set_child(GTK_FRAME(frame), box);
            gtk_widget_add_css_class(frame, "settings-group");
            gtk_widget_set_tooltip_text(frame, "PNG Settings");
            gtk_widget_set_visible(frame, true);
            gtk_widget_set_hexpand(frame, true);
            gtk_widget_set_vexpand(frame, true);

            lineBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_append(GTK_BOX(box), lineBox);
            label = gtk_label_new("Compression Level: ");
            gtk_box_append(GTK_BOX(lineBox), label);
            spinButton = gtk_spin_button_new_with_range(0, 9, 1);
            gtk_box_append(GTK_BOX(lineBox), spinButton);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinButton), 1);
            gtk_widget_set_tooltip_text(spinButton, "Compression Level");

            frame = gtk_frame_new("JPEG Settings");
            gtk_box_append(GTK_BOX(parentBox), frame);
            box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_frame_set_child(GTK_FRAME(frame), box);
            gtk_widget_add_css_class(frame, "settings-group");
            gtk_widget_set_tooltip_text(frame, "JPEG Settings");
            gtk_widget_set_visible(frame, true);
            gtk_widget_set_hexpand(frame, true);
            gtk_widget_set_vexpand(frame, true);

            lineBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_append(GTK_BOX(box), lineBox);
            label = gtk_label_new("Quality: ");
            gtk_box_append(GTK_BOX(lineBox), label);
            spinButton = gtk_spin_button_new_with_range(1, 100, 1);
            gtk_box_append(GTK_BOX(lineBox), spinButton);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinButton), 75);
            gtk_widget_set_tooltip_text(spinButton, "Quality");
        }

    public:
        PreferencesView()
        {
            m_RootElement = gtk_notebook_new();
            gtk_notebook_set_scrollable(GTK_NOTEBOOK(m_RootElement), true);
            gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_RootElement), true);
            gtk_notebook_set_tab_pos(GTK_NOTEBOOK(m_RootElement), GTK_POS_LEFT);
            gtk_widget_set_vexpand(GTK_WIDGET(m_RootElement), true);

            auto generalBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            auto label = gtk_label_new("General Settings");
            gtk_notebook_append_page(GTK_NOTEBOOK(m_RootElement), generalBox, label);
            gtk_box_append(GTK_BOX(generalBox), gtk_label_new("General Settings"));

            auto fileSettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            label = gtk_label_new("File Formats");
            gtk_notebook_append_page(GTK_NOTEBOOK(m_RootElement), fileSettingsBox, label);
            BuildFileSettingsBox(fileSettingsBox);
        }

        ~PreferencesView() override = default;

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootElement;
        }

        void Update(uint64_t lastSeenVersion) override
        {
            // Update logic for the preferences view
        }
    };

}
