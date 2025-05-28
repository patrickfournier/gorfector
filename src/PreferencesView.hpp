#pragma once

#include <adwaita.h>
#include <vector>

#include "App.hpp"
#include "Commands/DevMode/SetDumpSaneOptions.hpp"
#include "Commands/SetJpegQuality.hpp"
#include "Commands/SetPngCompressionLevel.hpp"
#include "Commands/SetTiffCompression.hpp"
#include "Commands/SetTiffDeflateLevel.hpp"
#include "Commands/SetTiffJpegQuality.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/SignalSupport.hpp"
#include "ZooLib/View.hpp"


namespace Gorfector
{
    class DeviceSelectorState;
    class JpegWriterState;
    class PngWriterState;
    class TiffWriterState;

    /**
     * \class PreferencesView
     * \brief Manages the preferences view of the application, including settings for image formats and developer
     * options.
     *
     * This class is responsible for building and managing the preferences UI, handling user interactions,
     * and dispatching commands to update the application state.
     */
    class PreferencesView : public ZooLib::View
    {
        /**
         * \brief Pointer to the main application instance.
         */
        App *m_App;

        /**
         * \brief Command dispatcher for handling user actions.
         */
        ZooLib::CommandDispatcher m_Dispatcher;

        /**
         * \brief Array of preference pages in the UI.
         */
        GtkWidget *m_PreferencesPages[2]{};

        /**
         * \brief Component managing TIFF writer state.
         */
        TiffWriterState *m_TiffWriterStateComponent{};

        /**
         * \brief Component managing PNG writer state.
         */
        PngWriterState *m_PngWriterStateComponent{};

        /**
         * \brief Component managing JPEG writer state.
         */
        JpegWriterState *m_JpegWriterStateComponent{};

        /**
         * \brief Component managing device selector state.
         */
        DeviceSelectorState *m_DeviceSelectorState;

        /**
         * \brief UI element for selecting TIFF compression algorithm.
         */
        GtkWidget *m_TiffCompressionAlgo{};

        /**
         * \brief UI element for setting TIFF deflate compression level.
         */
        GtkWidget *m_TiffDeflateCompressionLevel{};

        /**
         * \brief UI element for setting TIFF JPEG quality.
         */
        GtkWidget *m_TiffJpegQuality{};

        /**
         * \brief UI element for setting PNG compression level.
         */
        GtkWidget *m_PngCompressionLevel{};

        /**
         * \brief UI element for setting JPEG quality.
         */
        GtkWidget *m_JpegQuality{};

        /**
         * \brief UI element for enabling dumping of SANE options to stdout.
         */
        GtkWidget *m_DumpSaneOptions{};

        /**
         * \brief Observer for updating the view based on state changes.
         */
        ViewUpdateObserver<PreferencesView, TiffWriterState, JpegWriterState, PngWriterState> *m_ViewUpdateObserver;

        /**
         * \brief Constructs the PreferencesView.
         *
         * \param app The main application instance.
         * \param parentDispatcher The parent command dispatcher.
         * \param tiffWriterStateComponent The TIFF writer state component.
         * \param pngWriterStateComponent The PNG writer state component.
         * \param jpegWriterStateComponent The JPEG writer state component.
         * \param deviceSelectorState The device selector state component.
         */
        PreferencesView(
                App *app, ZooLib::CommandDispatcher *parentDispatcher, TiffWriterState *tiffWriterStateComponent,
                PngWriterState *pngWriterStateComponent, JpegWriterState *jpegWriterStateComponent,
                DeviceSelectorState *deviceSelectorState)
            : m_App(app)
            , m_Dispatcher(parentDispatcher)
            , m_TiffWriterStateComponent(tiffWriterStateComponent)
            , m_PngWriterStateComponent(pngWriterStateComponent)
            , m_JpegWriterStateComponent(jpegWriterStateComponent)
            , m_DeviceSelectorState(deviceSelectorState)
        {
            auto i = 0;
            m_PreferencesPages[i] = adw_preferences_page_new();
            adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[i]), _("Image Formats"));
            adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_PreferencesPages[i]), "emblem-photos-symbolic");
            BuildFileSettingsBox(m_PreferencesPages[i]);

            ++i;
            if (m_App->GetAppState()->IsDeveloperMode())
            {
                m_PreferencesPages[i] = adw_preferences_page_new();
                adw_preferences_page_set_title(ADW_PREFERENCES_PAGE(m_PreferencesPages[i]), _("Developer"));
                adw_preferences_page_set_icon_name(ADW_PREFERENCES_PAGE(m_PreferencesPages[i]), "diagnostics-symbolic");
                BuildDeveloperSettingsBox(m_PreferencesPages[i]);
            }
            else
            {
                m_PreferencesPages[i] = nullptr;
            }

            m_ViewUpdateObserver = new ViewUpdateObserver(
                    this, m_TiffWriterStateComponent, m_JpegWriterStateComponent, m_PngWriterStateComponent);
            app->GetObserverManager()->AddObserver(m_ViewUpdateObserver);
        }

        /**
         * \brief Builds the file settings section of the preferences UI.
         *
         * \param parent The parent widget to which the settings box will be added.
         */
        void BuildFileSettingsBox(GtkWidget *parent);

        /**
         * \brief Handles the selection of a TIFF compression algorithm.
         *
         * \param widget The widget triggering the event.
         */
        void OnCompressionAlgoSelected(GtkWidget *widget)
        {
            auto selectedIndex = static_cast<int>(adw_combo_row_get_selected(ADW_COMBO_ROW(widget)));
            m_Dispatcher.Dispatch(SetTiffCompression(selectedIndex));
        }

        /**
         * \brief Handles changes to spin row values in the preferences UI.
         *
         * \param widget The widget triggering the event.
         */
        void OnValueChanged(GtkWidget *widget)
        {
            auto value = static_cast<int>(adw_spin_row_get_value(ADW_SPIN_ROW(widget)));

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

        /**
         * \brief Builds the developer settings section of the preferences UI.
         *
         * \param parent The parent widget to which the settings box will be added.
         */
        void BuildDeveloperSettingsBox(GtkWidget *parent)
        {
            auto prefGroup = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(prefGroup), _("General"));
            adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(prefGroup));

            m_DumpSaneOptions = adw_switch_row_new();
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_DumpSaneOptions), _("Dump SANE options"));
            adw_action_row_set_subtitle(
                    ADW_ACTION_ROW(m_DumpSaneOptions),
                    _("When selecting a scanner, dump its SANE options to stdout. This is "
                      "useful for creating scanner config files."));
            adw_preferences_group_add(ADW_PREFERENCES_GROUP(prefGroup), m_DumpSaneOptions);
            ZooLib::ConnectGtkSignalWithParamSpecs(
                    this, &PreferencesView::OnDumpSaneOptionsChanged, m_DumpSaneOptions, "notify::active");

            m_Dispatcher.RegisterHandler(SetDumpSaneOptions::Execute, m_DeviceSelectorState);
        }

        /**
         * \brief Handles changes to the "Dump SANE options" switch.
         *
         * \param widget The widget triggering the event.
         */
        void OnDumpSaneOptionsChanged(GtkWidget *widget)
        {
            m_Dispatcher.Dispatch(SetDumpSaneOptions(adw_switch_row_get_active(ADW_SWITCH_ROW(widget))));
        }

    public:
        /**
         * \brief Creates a new instance of a `PreferencesView` class.
         *
         * This static method allocates and initializes a new `PreferencesView` instance, ensuring that
         * the `PostCreateView` method is called to set up the destroy signal.
         *
         * \param app The main application instance.
         * \param parentDispatcher The parent command dispatcher.
         * \param tiffWriterStateComponent The TIFF writer state component.
         * \param pngWriterStateComponent The PNG writer state component.
         * \param jpegWriterStateComponent The JPEG writer state component.
         * \param deviceSelectorState The device selector state component.
         * \return A pointer to the newly created `PreferencesView` instance.
         */
        static PreferencesView *
        Create(App *app, ZooLib::CommandDispatcher *parentDispatcher, TiffWriterState *tiffWriterStateComponent,
               PngWriterState *pngWriterStateComponent, JpegWriterState *jpegWriterStateComponent,
               DeviceSelectorState *deviceSelectorState)
        {
            auto view = new PreferencesView(
                    app, parentDispatcher, tiffWriterStateComponent, pngWriterStateComponent, jpegWriterStateComponent,
                    deviceSelectorState);
            view->PostCreateView();
            return view;
        }

        /**
         * \brief Destructor for PreferencesView.
         */
        ~PreferencesView() override
        {
            m_Dispatcher.UnregisterHandler<SetTiffCompression>();
            m_Dispatcher.UnregisterHandler<SetTiffDeflateLevel>();
            m_Dispatcher.UnregisterHandler<SetTiffJpegQuality>();
            m_Dispatcher.UnregisterHandler<SetPngCompressionLevel>();
            m_Dispatcher.UnregisterHandler<SetJpegQuality>();
            m_Dispatcher.UnregisterHandler<SetDumpSaneOptions>();

            m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);
            delete m_ViewUpdateObserver;

            m_ViewUpdateObserver = nullptr;
        }

        /**
         * \brief Gets the root widget of the preferences view.
         *
         * \return The root widget.
         */
        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_PreferencesPages[0];
        }

        /**
         * \brief Gets all preference pages in the view.
         *
         * \return A vector of preference page widgets.
         */
        [[nodiscard]] std::vector<GtkWidget *> GetPreferencePages() const
        {
            auto numPages = sizeof(m_PreferencesPages) / sizeof(m_PreferencesPages[0]);
            std::vector<GtkWidget *> pages;
            pages.reserve(numPages);

            for (auto i = 0UZ; i < numPages; ++i)
            {
                if (m_PreferencesPages[i] == nullptr)
                {
                    continue;
                }

                pages.push_back(m_PreferencesPages[i]);
            }

            return pages;
        }

        /**
         * \brief Updates the preferences view based on the latest state versions.
         *
         * \param lastSeenVersions A vector of the last seen state versions.
         */
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

            if (m_DumpSaneOptions != nullptr)
            {
                adw_switch_row_set_active(
                        ADW_SWITCH_ROW(m_DumpSaneOptions), m_DeviceSelectorState->IsDumpSaneEnabled());
            }
        }
    };
}
