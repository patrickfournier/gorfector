#include "PreferencesView.hpp"

void Gorfector::PreferencesView::BuildFileSettingsBox(GtkWidget *parent)
{
    auto prefGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), _("TIFF Settings"));
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

    m_TiffCompressionAlgo = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_TiffCompressionAlgo), _("Compression Algorithm"));
    auto algos = TiffWriterState::GetCompressionAlgorithmNames();
    adw_combo_row_set_model(ADW_COMBO_ROW(m_TiffCompressionAlgo), G_LIST_MODEL(gtk_string_list_new(algos.data())));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffCompressionAlgo);
    ZooLib::ConnectGtkSignalWithParamSpecs(
            this, &PreferencesView::OnCompressionAlgoSelected, m_TiffCompressionAlgo, "notify::selected");

    m_TiffDeflateCompressionLevel = adw_spin_row_new_with_range(0, 9, 1);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_TiffDeflateCompressionLevel), _("Deflate Compression Level"));
    adw_action_row_set_subtitle(
            ADW_ACTION_ROW(m_TiffDeflateCompressionLevel),
            _("0 = no compression, 9 = maximum compression. Higher compression levels may slow down the "
              "scanning process."));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffDeflateCompressionLevel);
    ZooLib::ConnectGtkSignalWithParamSpecs(
            this, &PreferencesView::OnValueChanged, m_TiffDeflateCompressionLevel, "notify::value");

    m_TiffJpegQuality = adw_spin_row_new_with_range(0, 100, 1);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_TiffJpegQuality), _("JPEG Quality"));
    adw_action_row_set_subtitle(ADW_ACTION_ROW(m_TiffJpegQuality), _("0 = lowest quality, 100 = maximum quality."));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffJpegQuality);
    ZooLib::ConnectGtkSignalWithParamSpecs(this, &PreferencesView::OnValueChanged, m_TiffJpegQuality, "notify::value");

    prefGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), _("PNG Settings"));
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

    m_PngCompressionLevel = adw_spin_row_new_with_range(0, 9, 1);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_PngCompressionLevel), _("Compression Level"));
    adw_action_row_set_subtitle(
            ADW_ACTION_ROW(m_PngCompressionLevel), _("0 = no compression, 9 = maximum compression. Higher "
                                                     "compression levels may slow down the scanning process."));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_PngCompressionLevel);
    ZooLib::ConnectGtkSignalWithParamSpecs(
            this, &PreferencesView::OnValueChanged, m_PngCompressionLevel, "notify::value");

    prefGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), _("JPEG Settings"));
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

    m_JpegQuality = adw_spin_row_new_with_range(0, 100, 1);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_JpegQuality), _("Quality"));
    adw_action_row_set_subtitle(ADW_ACTION_ROW(m_JpegQuality), _("0 = lowest quality, 100 = maximum quality."));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_JpegQuality);
    ZooLib::ConnectGtkSignalWithParamSpecs(this, &PreferencesView::OnValueChanged, m_JpegQuality, "notify::value");

    m_Dispatcher.RegisterHandler(SetTiffCompression::Execute, m_TiffWriterStateComponent);
    m_Dispatcher.RegisterHandler(SetTiffDeflateLevel::Execute, m_TiffWriterStateComponent);
    m_Dispatcher.RegisterHandler(SetTiffJpegQuality::Execute, m_TiffWriterStateComponent);
    m_Dispatcher.RegisterHandler(SetPngCompressionLevel::Execute, m_PngWriterStateComponent);
    m_Dispatcher.RegisterHandler(SetJpegQuality::Execute, m_JpegWriterStateComponent);
}
