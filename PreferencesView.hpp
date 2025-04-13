#pragma once

#include "ZooLib/View.hpp"

#include <Writers/TiffWriterStateComponent.hpp>

namespace ZooScan
{

    class PreferencesView : public ZooLib::View
    {
        GtkWidget *m_PreferencesPages[2]{};

        static void BuildFileSettingsBox(GtkWidget *parent)
        {
            auto prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "TIFF Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            auto line = adw_combo_row_new();
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(line), "Compression Algorithm");
            auto algos = TiffWriterStateComponent::GetCompressionAlgorithmNames();
            adw_combo_row_set_model(ADW_COMBO_ROW(line), G_LIST_MODEL(gtk_string_list_new(algos.data())));
            adw_combo_row_set_selected(
                    ADW_COMBO_ROW(line), TiffWriterStateComponent::GetDefaultCompressionAlgorithmIndex());
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), line);

            line = adw_spin_row_new_with_range(0, 9, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(line), "Deflate Compression Level");
            adw_spin_row_set_value(ADW_SPIN_ROW(line), 1);
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(line), "0 = no compression, 9 = maximum compression. Higher compression levels may "
                                          "slow down the scanning process.");
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), line);

            line = adw_spin_row_new_with_range(0, 100, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(line), "JPEG Quality");
            adw_action_row_set_subtitle(ADW_ACTION_ROW(line), "0 = lowest quality, 100 = maximum quality.");
            adw_spin_row_set_value(ADW_SPIN_ROW(line), 75);
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), line);

            prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "PNG Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            line = adw_spin_row_new_with_range(0, 9, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(line), "Compression Level");
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(line), "0 = no compression, 9 = maximum compression. Higher compression levels may "
                                          "slow down the scanning process.");
            adw_spin_row_set_value(ADW_SPIN_ROW(line), 1);
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), line);

            prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "JPEG Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            line = adw_spin_row_new_with_range(0, 100, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(line), "Quality");
            adw_action_row_set_subtitle(ADW_ACTION_ROW(line), "0 = lowest quality, 100 = maximum quality.");
            adw_spin_row_set_value(ADW_SPIN_ROW(line), 75);
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), line);
        }

    public:
        PreferencesView()
        {
            m_PreferencesPages[0] = adw_preferences_page_new();
            adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[0]), "General");
            adw_preferences_page_set_icon_name(
                    ADW_PREFERENCES_PAGE(m_PreferencesPages[0]), "preferences-system-symbolic");

            m_PreferencesPages[1] = adw_preferences_page_new();
            adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[1]), "File Formats");
            adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_PreferencesPages[1]), "document-save-symbolic");
            BuildFileSettingsBox(m_PreferencesPages[1]);
        }

        ~PreferencesView() override = default;

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_PreferencesPages[0];
        }

        [[nodiscard]] std::vector<GtkWidget *> GetPreferencePages() const
        {
            return {m_PreferencesPages[0], m_PreferencesPages[1]};
        }

        void Update(uint64_t lastSeenVersion) override
        {
            // Update logic for the preferences view
        }
    };

}
