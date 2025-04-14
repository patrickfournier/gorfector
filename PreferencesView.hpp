#pragma once

#include <adwaita.h>

#include "Commands/SetJpegQuality.hpp"
#include "Commands/SetPngCompressionLevel.hpp"
#include "Commands/SetTiffCompression.hpp"
#include "Commands/SetTiffDeflateLevel.hpp"
#include "Commands/SetTiffJpegQuality.hpp"
#include "ViewUpdateObserver.hpp"
#include "Writers/JpegWriterState.hpp"
#include "Writers/PngWriterState.hpp"
#include "Writers/TiffWriterState.hpp"
#include "ZooLib/View.hpp"

namespace ZooScan
{

    class PreferencesView : public ZooLib::View
    {
        App *m_App;
        ZooLib::CommandDispatcher m_Dispatcher;

        GtkWidget *m_PreferencesPages[2]{};

        TiffWriterState *m_TiffWriterStateComponent{};
        PngWriterState *m_PngWriterStateComponent{};
        JpegWriterState *m_JpegWriterStateComponent{};

        GtkWidget *m_TiffCompressionAlgo{};
        GtkWidget *m_TiffDeflateCompressionLevel{};
        GtkWidget *m_TiffJpegQuality{};
        GtkWidget *m_PngCompressionLevel{};
        GtkWidget *m_JpegQuality{};

        ViewUpdateObserver<PreferencesView, TiffWriterState, JpegWriterState, PngWriterState> *m_ViewUpdateObserver;

        void BuildFileSettingsBox(GtkWidget *parent)
        {
            auto prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "TIFF Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            m_TiffCompressionAlgo = adw_combo_row_new();
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_TiffCompressionAlgo), "Compression Algorithm");
            auto algos = TiffWriterState::GetCompressionAlgorithmNames();
            adw_combo_row_set_model(
                    ADW_COMBO_ROW(m_TiffCompressionAlgo), G_LIST_MODEL(gtk_string_list_new(algos.data())));
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffCompressionAlgo);
            ConnectGtkSignalWithParamSpecs(
                    this, &PreferencesView::OnCompressionAlgoSelected, m_TiffCompressionAlgo, "notify::selected");

            m_TiffDeflateCompressionLevel = adw_spin_row_new_with_range(0, 9, 1);
            adw_preferences_row_set_title(
                    ADW_PREFERENCES_ROW(m_TiffDeflateCompressionLevel), "Deflate Compression Level");
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(m_TiffDeflateCompressionLevel),
                    "0 = no compression, 9 = maximum compression. Higher compression levels may "
                    "slow down the scanning process.");
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffDeflateCompressionLevel);
            ConnectGtkSignalWithParamSpecs(
                    this, &PreferencesView::OnValueChanged, m_TiffDeflateCompressionLevel, "notify::value");

            m_TiffJpegQuality = adw_spin_row_new_with_range(0, 100, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_TiffJpegQuality), "JPEG Quality");
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(m_TiffJpegQuality), "0 = lowest quality, 100 = maximum quality.");
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_TiffJpegQuality);
            ConnectGtkSignalWithParamSpecs(this, &PreferencesView::OnValueChanged, m_TiffJpegQuality, "notify::value");

            prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "PNG Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            m_PngCompressionLevel = adw_spin_row_new_with_range(0, 9, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_PngCompressionLevel), "Compression Level");
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(m_PngCompressionLevel),
                    "0 = no compression, 9 = maximum compression. Higher compression levels may "
                    "slow down the scanning process.");
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_PngCompressionLevel);
            ConnectGtkSignalWithParamSpecs(
                    this, &PreferencesView::OnValueChanged, m_PngCompressionLevel, "notify::value");

            prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), "JPEG Settings");
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            m_JpegQuality = adw_spin_row_new_with_range(0, 100, 1);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_JpegQuality), "Quality");
            adw_action_row_set_subtitle(ADW_ACTION_ROW(m_JpegQuality), "0 = lowest quality, 100 = maximum quality.");
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_JpegQuality);
            ConnectGtkSignalWithParamSpecs(this, &PreferencesView::OnValueChanged, m_JpegQuality, "notify::value");
        }

        void OnCompressionAlgoSelected(GtkWidget *widget)
        {
            auto selectedIndex = adw_combo_row_get_selected(ADW_COMBO_ROW(widget));
            m_Dispatcher.Dispatch(SetTiffCompression(selectedIndex));
        }

        void OnValueChanged(GtkWidget *widget)
        {
            auto value = adw_spin_row_get_value(ADW_SPIN_ROW(widget));

            if (widget == m_TiffDeflateCompressionLevel)
            {
                m_Dispatcher.Dispatch(SetTiffDeflateLevel(value));
            }
            else if (widget == m_TiffJpegQuality)
            {
                m_Dispatcher.Dispatch(SetTiffJpegQuality(value));
            }
            else if (widget == m_PngCompressionLevel)
            {
                m_Dispatcher.Dispatch(SetPngCompressionLevel(value));
            }
            else if (widget == m_JpegQuality)
            {
                m_Dispatcher.Dispatch(SetJpegQuality(value));
            }
        }

    public:
        PreferencesView(
                App *app, ZooLib::CommandDispatcher *parentDispatcher, TiffWriterState *tiffWriterStateComponent,
                PngWriterState *pngWriterStateComponent, JpegWriterState *jpegWriterStateComponent)
            : m_App(app)
            , m_Dispatcher(parentDispatcher)
            , m_TiffWriterStateComponent(tiffWriterStateComponent)
            , m_PngWriterStateComponent(pngWriterStateComponent)
            , m_JpegWriterStateComponent(jpegWriterStateComponent)
        {
            m_PreferencesPages[0] = adw_preferences_page_new();
            adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[0]), "General");
            adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_PreferencesPages[0]), "configure-symbolic");

            m_PreferencesPages[1] = adw_preferences_page_new();
            adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[1]), "Image Formats");
            adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_PreferencesPages[1]), "emblem-photos-symbolic");
            BuildFileSettingsBox(m_PreferencesPages[1]);

            m_ViewUpdateObserver = new ViewUpdateObserver(
                    this, m_TiffWriterStateComponent, m_JpegWriterStateComponent, m_PngWriterStateComponent);
            app->GetObserverManager()->AddObserver(m_ViewUpdateObserver);

            m_Dispatcher.RegisterHandler<SetTiffCompression, TiffWriterState>(
                    SetTiffCompression::Execute, m_TiffWriterStateComponent);
            m_Dispatcher.RegisterHandler<SetTiffDeflateLevel, TiffWriterState>(
                    SetTiffDeflateLevel::Execute, m_TiffWriterStateComponent);
            m_Dispatcher.RegisterHandler<SetTiffJpegQuality, TiffWriterState>(
                    SetTiffJpegQuality::Execute, m_TiffWriterStateComponent);
            m_Dispatcher.RegisterHandler<SetPngCompressionLevel, PngWriterState>(
                    SetPngCompressionLevel::Execute, m_PngWriterStateComponent);
            m_Dispatcher.RegisterHandler<SetJpegQuality, JpegWriterState>(
                    SetJpegQuality::Execute, m_JpegWriterStateComponent);
        }

        ~PreferencesView() override
        {
            m_Dispatcher.UnregisterHandler<SetTiffCompression>();
            m_Dispatcher.UnregisterHandler<SetTiffDeflateLevel>();
            m_Dispatcher.UnregisterHandler<SetTiffJpegQuality>();
            m_Dispatcher.UnregisterHandler<SetPngCompressionLevel>();
            m_Dispatcher.UnregisterHandler<SetJpegQuality>();

            m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);
            delete m_ViewUpdateObserver;
        }

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_PreferencesPages[0];
        }

        [[nodiscard]] std::vector<GtkWidget *> GetPreferencePages() const
        {
            return {m_PreferencesPages[0], m_PreferencesPages[1]};
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions) override
        {
            adw_combo_row_set_selected(
                    ADW_COMBO_ROW(m_TiffCompressionAlgo), m_TiffWriterStateComponent->GetCompressionIndex());
            adw_spin_row_set_value(
                    ADW_SPIN_ROW(m_TiffDeflateCompressionLevel),
                    m_TiffWriterStateComponent->GetDeflateCompressionLevel());
            adw_spin_row_set_value(ADW_SPIN_ROW(m_TiffJpegQuality), m_TiffWriterStateComponent->GetJpegQuality());
            adw_spin_row_set_value(
                    ADW_SPIN_ROW(m_PngCompressionLevel), m_PngWriterStateComponent->GetCompressionLevel());
            adw_spin_row_set_value(ADW_SPIN_ROW(m_JpegQuality), m_JpegWriterStateComponent->GetQuality());
        }
    };

}
