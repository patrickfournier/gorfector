#include <adwaita.h>
#include <format>
#include <regex>
#include <sane/sane.h>
#include <stdexcept>

#include "App.hpp"
#include "AppState.hpp"
#include "Commands/CreateScanListItemCommand.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "Commands/ToggleUseScanList.hpp"
#include "DeviceOptionsObserver.hpp"
#include "DeviceSelector.hpp"
#include "DeviceSelectorObserver.hpp"
#include "OutputOptionsState.hpp"
#include "PreferencesView.hpp"
#include "PresetPanel.hpp"
#include "PreviewPanel.hpp"
#include "ScanListPanel.hpp"
#include "ScanOptionsPanel.hpp"
#include "Writers/FileWriter.hpp"
#include "Writers/JpegWriter.hpp"
#include "Writers/PngWriter.hpp"
#include "Writers/TiffWriter.hpp"
#include "ZooLib/AppMenuBarBuilder.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/SignalSupport.hpp"

Gorfector::App::App(int argc, char **argv)
{
    SANE_Int saneVersion;
    if (SANE_STATUS_GOOD != sane_init(&saneVersion, nullptr))
    {
        sane_exit();
        throw std::runtime_error("Failed to initialize SANE");
    }

    auto prefDir = std::filesystem::path(g_get_user_config_dir()) / k_ApplicationId;
    if (!std::filesystem::exists(prefDir))
    {
        std::filesystem::create_directories(prefDir);
    }
    auto prefFilePath = prefDir / "preferences.json";
    m_State.SetPreferenceFilePath(prefFilePath);

    bool devMode{};
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--dev") == 0)
        {
            devMode = true;
        }
    }

    m_AppState = new AppState(&m_State, devMode);
    m_State.LoadFromPreferenceFile(m_AppState);

    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_ViewUpdateObserver);

    m_DeviceSelectorState = new DeviceSelectorState(&m_State);
    m_DeviceSelectorObserver = new DeviceSelectorObserver(m_DeviceSelectorState, m_AppState);
    m_ObserverManager.AddObserver(m_DeviceSelectorObserver);

    FileWriter::Register<TiffWriter>(&m_State);
    FileWriter::Register<JpegWriter>(&m_State);
    FileWriter::Register<PngWriter>(&m_State);
}

Gorfector::App::~App()
{
    m_Dispatcher.UnregisterHandler<SetScanAreaCommand>();

    m_ObserverManager.RemoveObserver(m_DeviceSelectorObserver);
    delete m_DeviceSelectorObserver;

    // m_DeviceOptionsPanel and m_PreviewPanel are automatically deleted when their root widget is destroyed.

    m_ObserverManager.RemoveObserver(m_ViewUpdateObserver);
    delete m_ViewUpdateObserver;

    delete m_AppState;
    delete m_DeviceSelectorState;

    if (m_Buffer != nullptr)
    {
        free(m_Buffer);
    }

    sane_exit();

    FileWriter::Clear();
}

void Gorfector::App::OnActivate(GtkApplication *app)
{
    Application::OnActivate(app);

    if (GetSelectorDeviceName().empty())
    {
        auto action = g_action_map_lookup_action(G_ACTION_MAP(app), "select_device");
        g_action_activate(action, nullptr);
    }
}

void Gorfector::App::OnPanelResized(GtkWidget *widget)
{
    auto windowWidth = gtk_widget_get_width(m_MainWindow);
    auto position = gtk_paned_get_position(GTK_PANED(widget));
    if (widget == m_LeftPaned)
    {
        auto updater = AppState::Updater(m_AppState);
        updater.SetLeftPanelWidth(static_cast<double>(position) / windowWidth);
    }
    else if (widget == m_RightPaned)
    {
        auto updater = AppState::Updater(m_AppState);
        updater.SetRightPanelWidth(static_cast<double>(position) / windowWidth);
    }
}

GtkWidget *Gorfector::App::BuildUI()
{
    auto *display = gdk_display_get_default();
    auto *cssProvider = gtk_css_provider_new();
    std::string cssPath = std::string("/com/patrickfournier/gorfector/resources/gorfector.css");
    gtk_css_provider_load_from_resource(cssProvider, cssPath.c_str());
    gtk_style_context_add_provider_for_display(
            display, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
    g_object_unref(cssProvider);

    auto deviceSelected = !GetSelectorDeviceName().empty();

    m_LeftPaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_bottom(m_LeftPaned, 0);
    gtk_widget_set_margin_top(m_LeftPaned, 0);
    gtk_widget_set_margin_start(m_LeftPaned, 10);
    gtk_widget_set_margin_end(m_LeftPaned, 10);
    gtk_paned_set_wide_handle(GTK_PANED(m_LeftPaned), true);
    auto windowWidth = gtk_widget_get_width(m_MainWindow);
    windowWidth = windowWidth == 0 ? std::get<0>(GetMainWindowSize()) : windowWidth;
    gtk_paned_set_position(GTK_PANED(m_LeftPaned), static_cast<int>(windowWidth * m_AppState->GetLeftPanelWidth()));
    ZooLib::ConnectGtkSignalWithParamSpecs(this, &App::OnPanelResized, m_LeftPaned, "notify::position");

    auto clamp = adw_clamp_new();
    adw_clamp_set_unit(ADW_CLAMP(clamp), ADW_LENGTH_UNIT_SP);
    adw_clamp_set_tightening_threshold(ADW_CLAMP(clamp), 600);
    adw_clamp_set_maximum_size(ADW_CLAMP(clamp), 800);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(clamp), GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(m_LeftPaned), clamp);

    m_SettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(m_SettingsBox, 0);
    gtk_widget_set_margin_top(m_SettingsBox, 0);
    gtk_widget_set_margin_start(m_SettingsBox, 0);
    gtk_widget_set_margin_end(m_SettingsBox, 10);
    adw_clamp_set_child(ADW_CLAMP(clamp), m_SettingsBox);

    auto buttonBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_bottom(buttonBox, 20);
    gtk_widget_set_margin_top(buttonBox, 20);
    gtk_widget_set_margin_start(buttonBox, 0);
    gtk_widget_set_margin_end(buttonBox, 0);
    gtk_box_append(GTK_BOX(m_SettingsBox), buttonBox);

    m_PreviewButton = gtk_button_new_with_label(_("Preview"));
    ConnectGtkSignal(this, &App::OnPreviewClicked, m_PreviewButton, "clicked");
    gtk_widget_set_sensitive(m_PreviewButton, deviceSelected);
    gtk_box_append(GTK_BOX(buttonBox), m_PreviewButton);

    m_ScanButton = gtk_button_new_with_label(_("Scan"));
    ConnectGtkSignal(this, &App::OnScanClicked, m_ScanButton, "clicked");
    gtk_widget_set_sensitive(m_ScanButton, deviceSelected);
    gtk_box_append(GTK_BOX(buttonBox), m_ScanButton);

    m_CancelButton = gtk_button_new_with_label(_("Cancel Scan"));
    ConnectGtkSignal(this, &App::OnCancelClicked, m_CancelButton, "clicked");
    gtk_widget_set_sensitive(m_CancelButton, false);
    gtk_box_append(GTK_BOX(buttonBox), m_CancelButton);

    m_AddToScanListButton = gtk_button_new_with_label(_("Add to Scan List"));
    ConnectGtkSignal(this, &App::OnAddToScanListClicked, m_AddToScanListButton, "clicked");
    gtk_widget_set_sensitive(m_AddToScanListButton, deviceSelected);
    gtk_box_append(GTK_BOX(buttonBox), m_AddToScanListButton);

    auto bottomAlignedSection = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(bottomAlignedSection, true);
    gtk_widget_set_vexpand(bottomAlignedSection, true);
    gtk_widget_set_valign(bottomAlignedSection, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(m_SettingsBox), bottomAlignedSection);

    m_PresetPanel = ZooLib::View::Create<PresetPanel>(&m_Dispatcher, this);
    auto presetsBox = m_PresetPanel->GetRootWidget();
    gtk_widget_set_margin_bottom(presetsBox, 0);
    gtk_widget_set_margin_top(presetsBox, 0);
    gtk_widget_set_margin_start(presetsBox, 0);
    gtk_widget_set_margin_end(presetsBox, 0);
    gtk_box_append(GTK_BOX(bottomAlignedSection), presetsBox);
    gtk_widget_set_hexpand(presetsBox, true);

    m_PreviewPanel = ZooLib::View::Create<PreviewPanel>(&m_Dispatcher, this);
    auto previewBox = m_PreviewPanel->GetRootWidget();
    gtk_widget_set_margin_bottom(previewBox, 10);
    gtk_widget_set_margin_top(previewBox, 0);
    gtk_widget_set_margin_start(previewBox, 10);
    gtk_widget_set_margin_end(previewBox, 10);

    if (m_AppState->GetUseScanList())
    {
        BuildScanListUI();
    }
    else
    {
        gtk_paned_set_end_child(GTK_PANED(m_LeftPaned), previewBox);
    }

    return m_LeftPaned;
}

void Gorfector::App::BuildScanListUI()
{
    m_RightPaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_wide_handle(GTK_PANED(m_RightPaned), true);
    auto windowWidth = gtk_widget_get_width(m_MainWindow);
    windowWidth = windowWidth == 0 ? std::get<0>(GetMainWindowSize()) : windowWidth;
    gtk_paned_set_position(GTK_PANED(m_RightPaned), static_cast<int>(windowWidth * m_AppState->GetRightPanelWidth()));
    gtk_paned_set_end_child(GTK_PANED(m_LeftPaned), m_RightPaned);
    ZooLib::ConnectGtkSignalWithParamSpecs(this, &App::OnPanelResized, m_RightPaned, "notify::position");

    gtk_paned_set_start_child(GTK_PANED(m_RightPaned), m_PreviewPanel->GetRootWidget());

    auto clamp = adw_clamp_new();
    adw_clamp_set_unit(ADW_CLAMP(clamp), ADW_LENGTH_UNIT_SP);
    adw_clamp_set_tightening_threshold(ADW_CLAMP(clamp), 300);
    adw_clamp_set_maximum_size(ADW_CLAMP(clamp), 500);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(clamp), GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_end_child(GTK_PANED(m_RightPaned), clamp);

    m_ScanListPanel = ZooLib::View::Create<ScanListPanel>(&m_Dispatcher, this);
    auto scanListBox = m_ScanListPanel->GetRootWidget();
    adw_clamp_set_child(ADW_CLAMP(clamp), scanListBox);

    m_Dispatcher.RegisterHandler(CreateScanListItemCommand::Execute, m_ScanListPanel->GetState());
}

void Gorfector::App::RemoveScanListUI()
{
    m_RightPaned = nullptr;
    m_ScanListPanel = nullptr;
    m_Dispatcher.UnregisterHandler<CreateScanListItemCommand>();
}

void Gorfector::App::PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder)
{
    menuBarBuilder->BeginSection()
            ->AddMenuItem(_("Select Device..."), "app.select_device")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem("Use Scan List", "app.scanlist")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem(_("Settings..."), "app.preferences")
            ->AddMenuItem(_("About..."), "app.about")
            ->EndSection();

    BindMethodToAction<App>("select_device", &App::SelectDeviceDialog, this);
    BindMethodToAction<Application>("quit", &Application::Quit, this);
    BindCommandToToggleAction<ToggleUseScanList>("scanlist", m_AppState->GetUseScanList(), m_AppState);
    BindMethodToAction<App>("preferences", &App::PreferenceDialog, this);
    BindMethodToAction<App>("about", &App::AboutDialog, this);

    SetAcceleratorForAction("app.quit", {"<Ctrl>Q"});
    SetAcceleratorForAction("app.undo", {"<Ctrl>Z"});
    SetAcceleratorForAction("app.redo", {"<Ctrl><Shift>Z"});
}

void Gorfector::App::SelectDeviceDialog(GSimpleAction *action, GVariant *parameter)
{
    auto deviceSelector = ZooLib::View::Create<DeviceSelector>(&m_Dispatcher, this, m_DeviceSelectorState);
    auto dialog = adw_dialog_new();
    adw_dialog_set_title(dialog, _("Select Device"));
    adw_dialog_set_follows_content_size(dialog, true);

    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    auto header = adw_header_bar_new();
    gtk_box_append(GTK_BOX(box), header);
    gtk_box_append(GTK_BOX(box), deviceSelector->GetRootWidget());

    adw_dialog_set_child(dialog, box);
    adw_dialog_present(dialog, m_MainWindow);
}

void Gorfector::App::PreferenceDialog(GSimpleAction *action, GVariant *parameter)
{
    auto dialog = adw_preferences_dialog_new();
    auto preferencePages = ZooLib::View::Create<PreferencesView>(
            this, &m_Dispatcher, FileWriter::GetFormatByType<TiffWriter>()->GetStateComponent(),
            FileWriter::GetFormatByType<PngWriter>()->GetStateComponent(),
            FileWriter::GetFormatByType<JpegWriter>()->GetStateComponent(), m_DeviceSelectorState);
    for (const auto &page: preferencePages->GetPreferencePages())
    {
        adw_preferences_dialog_add(ADW_PREFERENCES_DIALOG(dialog), ADW_PREFERENCES_PAGE(page));
    }

    adw_dialog_set_title(dialog, _("Settings"));
    adw_dialog_present(dialog, m_MainWindow);
}

void Gorfector::App::AboutDialog(GSimpleAction *action, GVariant *parameter)
{
    auto dialogWindow = adw_about_dialog_new();
    adw_about_dialog_set_application_icon(
            ADW_ABOUT_DIALOG(dialogWindow),
            // FIXME should use the icon in resources, but does not work
            "scanner-symbolic");
    adw_about_dialog_set_application_name(ADW_ABOUT_DIALOG(dialogWindow), k_ApplicationName);
    adw_about_dialog_set_version(ADW_ABOUT_DIALOG(dialogWindow), "0.1");
    adw_about_dialog_set_comments(
            ADW_ABOUT_DIALOG(dialogWindow), _("An application to scan images.")); // FIXME: could be more detailed
    adw_about_dialog_set_license_type(ADW_ABOUT_DIALOG(dialogWindow), GTK_LICENSE_GPL_3_0);
    adw_about_dialog_set_developer_name(ADW_ABOUT_DIALOG(dialogWindow), "Patrick Fournier");

    adw_dialog_present(ADW_DIALOG(dialogWindow), m_MainWindow);
}

const std::string &Gorfector::App::GetSelectorDeviceName() const
{
    if (m_DeviceSelectorState == nullptr)
    {
        return DeviceSelectorState::k_NullDeviceName;
    }

    return m_DeviceSelectorState->GetSelectedDeviceName();
}

int Gorfector::App::GetSelectorSaneInitId() const
{
    if (m_DeviceSelectorState == nullptr)
    {
        return 0;
    }

    return m_DeviceSelectorState->GetSelectorSaneInitId();
}

void Gorfector::App::Update(const std::vector<uint64_t> &lastSeenVersions)
{
    auto changeset = m_AppState->GetAggregatedChangeset(lastSeenVersions[0]);
    if (changeset == nullptr || !changeset->HasAnyChange())
    {
        return;
    }

    if (m_ScanOptionsPanel != nullptr && changeset->IsChanged(AppStateChangeset::ChangeTypeFlag::e_CurrentDevice))
    {
        if (m_DeviceOptionsObserver != nullptr)
        {
            m_ObserverManager.RemoveObserver(m_DeviceOptionsObserver);
            delete m_DeviceOptionsObserver;
        }

        gtk_box_remove(GTK_BOX(m_SettingsBox), m_ScanOptionsPanel->GetRootWidget());
        // m_ScanOptionsPanel is automatically deleted when its root widget is destroyed.
        m_ScanOptionsPanel = nullptr;
        m_Dispatcher.UnregisterHandler<SetScanAreaCommand>();
    }

    if (m_ScanOptionsPanel == nullptr && m_SettingsBox != nullptr && !GetSelectorDeviceName().empty())
    {
        m_ScanOptionsPanel = ZooLib::View::Create<ScanOptionsPanel>(
                GetSelectorSaneInitId(), GetSelectorDeviceName(), &m_Dispatcher, this);
        gtk_box_prepend(GTK_BOX(m_SettingsBox), m_ScanOptionsPanel->GetRootWidget());
        m_Dispatcher.RegisterHandler(SetScanAreaCommand::Execute, m_ScanOptionsPanel->GetDeviceOptionsState());

        if (m_PreviewPanel != nullptr)
        {
            m_DeviceOptionsObserver =
                    new DeviceOptionsObserver(m_ScanOptionsPanel->GetDeviceOptionsState(), m_PreviewPanel->GetState());
            m_ObserverManager.AddObserver(m_DeviceOptionsObserver);
        }
    }

    if (changeset->IsChanged(AppStateChangeset::ChangeTypeFlag::e_ScanListMode))
    {
        // Update the action associated with the scan mode
        auto action = g_action_map_lookup_action(G_ACTION_MAP(m_GtkApp), "scanlist");
        g_action_change_state(action, g_variant_new_boolean(m_AppState->GetUseScanList()));

        gtk_widget_set_visible(m_ScanButton, !m_AppState->GetUseScanList());
        gtk_widget_set_visible(m_CancelButton, !m_AppState->GetUseScanList());
        gtk_widget_set_visible(m_AddToScanListButton, m_AppState->GetUseScanList());

        if (m_PreviewPanel != nullptr)
        {
            auto previewBox = m_PreviewPanel->GetRootWidget();
            auto previewBoxParent = gtk_widget_get_parent(previewBox);

            if (m_AppState->GetUseScanList() && previewBoxParent == m_LeftPaned)
            {
                g_object_ref(previewBox);
                gtk_widget_unparent(previewBox);
                BuildScanListUI();
                g_object_unref(previewBox);

                auto windowWidth = gtk_widget_get_width(m_MainWindow);
                windowWidth = windowWidth == 0 ? std::get<0>(GetMainWindowSize()) : windowWidth;
                gtk_paned_set_position(
                        GTK_PANED(m_RightPaned), static_cast<int>(windowWidth * m_AppState->GetRightPanelWidth()));
            }
            else if (!m_AppState->GetUseScanList() && previewBoxParent == m_RightPaned)
            {
                g_object_ref(previewBox);
                gtk_widget_unparent(previewBox);
                gtk_paned_set_end_child(GTK_PANED(m_LeftPaned), previewBox);
                g_object_unref(previewBox);

                RemoveScanListUI();
            }
        }
    }

    if (changeset->IsChanged(AppStateChangeset::ChangeTypeFlag::e_ScanActivity))
    {
        // TODO: also update the option panel
        if (m_AppState->IsScanning() || m_AppState->IsPreviewing())
        {
            gtk_widget_set_sensitive(m_PreviewButton, false);
            gtk_widget_set_sensitive(m_ScanButton, false);
            gtk_widget_set_sensitive(m_CancelButton, true);
            gtk_widget_set_sensitive(m_AddToScanListButton, false);
        }
        else if (!GetSelectorDeviceName().empty())
        {
            gtk_widget_set_sensitive(m_PreviewButton, true);
            gtk_widget_set_sensitive(m_ScanButton, true);
            gtk_widget_set_sensitive(m_CancelButton, false);
            gtk_widget_set_sensitive(m_AddToScanListButton, true);
        }
        else
        {
            gtk_widget_set_sensitive(m_PreviewButton, false);
            gtk_widget_set_sensitive(m_ScanButton, false);
            gtk_widget_set_sensitive(m_CancelButton, false);
            gtk_widget_set_sensitive(m_AddToScanListButton, false);
        }
    }
}

Gorfector::DeviceOptionsState *Gorfector::App::GetDeviceOptions() const
{
    if (m_ScanOptionsPanel == nullptr)
        return nullptr;

    return m_ScanOptionsPanel->GetDeviceOptionsState();
}

Gorfector::OutputOptionsState *Gorfector::App::GetOutputOptions()
{
    if (m_ScanOptionsPanel == nullptr)
        return nullptr;

    return m_ScanOptionsPanel->GetOutputOptionsState();
}

void Gorfector::App::RestoreOptionsAfterPreview()
{
    auto device = GetDevice();
    auto options = GetDeviceOptions();
    if (device == nullptr || options == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_FALSE;
    device->SetOptionValue(options->PreviewIndex(), &isPreview, nullptr);

    if (options->XResolutionIndex() == std::numeric_limits<uint32_t>::max())
    {
        if (options->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
        {
            auto option = options->GetOption<int>(options->ResolutionIndex());
            int value = option->GetValue();
            device->SetOptionValue(options->ResolutionIndex(), &value, nullptr);
        }
    }
    else
    {
        auto option = options->GetOption<int>(options->XResolutionIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->XResolutionIndex(), &value, nullptr);
    }

    if (options->YResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->YResolutionIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->YResolutionIndex(), &value, nullptr);
    }

    if (options->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->BitDepthIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->BitDepthIndex(), &value, nullptr);
    }

    if (options->TLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->TLXIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->TLXIndex(), &value, nullptr);
    }
    if (options->TLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->TLYIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->TLYIndex(), &value, nullptr);
    }
    if (options->BRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->BRXIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->BRXIndex(), &value, nullptr);
    }
    if (options->BRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->BRYIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->BRYIndex(), &value, nullptr);
    }
}

int Gorfector::App::GetScanHeight() const
{
    auto height = m_ScanParameters.lines;

    if (height == -1)
    {
        auto options = GetDeviceOptions();
        if (options != nullptr)
        {
            height = std::ceil(options->GetScanArea().height * options->GetYResolution() / 25.4);
        }
        else
        {
            height = 0;
        }
    }

    return height;
}

void Gorfector::App::OnPreviewClicked(GtkWidget *)
{
    auto device = GetDevice();
    auto options = GetDeviceOptions();
    if (device == nullptr || options == nullptr)
    {
        return;
    }

    // Set preview settings on the device

    SANE_Bool isPreview = SANE_TRUE;
    device->SetOptionValue(options->PreviewIndex(), &isPreview, nullptr);

    if (options->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto resolutionDescription = device->GetOptionDescriptor(options->ResolutionIndex());
        SANE_Int resolution = resolutionDescription->type == SANE_TYPE_FIXED ? SANE_FIX(300) : 300;
        if (resolutionDescription->constraint_type == SANE_CONSTRAINT_RANGE)
        {
            resolution = std::clamp(
                    resolution, resolutionDescription->constraint.range->min,
                    resolutionDescription->constraint.range->max);
        }
        else if (resolutionDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
        {
            auto count = resolutionDescription->constraint.word_list[0];
            int nearestResolutionIndex = 1;
            SANE_Int smallestDiff = std::abs(resolution - resolutionDescription->constraint.word_list[1]);
            for (auto i = 2; i <= count; i++)
            {
                auto diff = std::abs(resolution - resolutionDescription->constraint.word_list[1]);
                if (diff < smallestDiff)
                {
                    nearestResolutionIndex = i;
                    smallestDiff = diff;
                }
            }
            resolution = resolutionDescription->constraint.word_list[nearestResolutionIndex];
        }

        device->SetOptionValue(options->ResolutionIndex(), &resolution, nullptr);
    }

    if (options->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto depthDescription = device->GetOptionDescriptor(options->BitDepthIndex());

        if (!(depthDescription->cap & SANE_CAP_INACTIVE))
        {
            SANE_Int depth = depthDescription->type == SANE_TYPE_FIXED ? SANE_FIX(8) : 8;
            if (depthDescription->constraint_type == SANE_CONSTRAINT_RANGE)
            {
                depth = std::clamp(
                        depth, depthDescription->constraint.range->min, depthDescription->constraint.range->max);
            }
            else if (depthDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
            {
                auto count = depthDescription->constraint.word_list[0];
                SANE_Int minDepth = depthDescription->constraint.word_list[1];
                for (auto i = 1; i <= count; i++)
                {
                    if (depthDescription->constraint.word_list[i] == depth)
                    {
                        // If 8 bits is there, we take it.
                        break;
                    }

                    minDepth = std::min(minDepth, depthDescription->constraint.word_list[i]);
                }
            }

            device->SetOptionValue(options->BitDepthIndex(), &depth, nullptr);
        }
    }

    auto maxScanArea = options->GetMaxScanArea();
    if (options->TLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->TLXIndex());
        double value = maxScanArea.x;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        device->SetOptionValue(options->TLXIndex(), &fValue, nullptr);
    }
    if (options->TLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->TLYIndex());
        double value = maxScanArea.y;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        device->SetOptionValue(options->TLYIndex(), &fValue, nullptr);
    }
    if (options->BRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->BRXIndex());
        double value = maxScanArea.x + maxScanArea.width;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        device->SetOptionValue(options->BRXIndex(), &fValue, nullptr);
    }
    if (options->BRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->BRYIndex());
        double value = maxScanArea.y + maxScanArea.height;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        device->SetOptionValue(options->BRYIndex(), &fValue, nullptr);
    }

    try
    {
        device->StartScan();
        auto updater = AppState::Updater(m_AppState);
        updater.SetIsPreviewing(true);
    }
    catch (const std::runtime_error &e)
    {
        StopPreview();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), e.what());
        return;
    }

    device->GetParameters(&m_ScanParameters);
    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        StopPreview();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
        return;
    }

    if (m_PreviewPanel != nullptr)
    {
        auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
        previewPanelUpdater.PrepareForScan(
                m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, m_ScanParameters.lines,
                m_ScanParameters.depth, m_ScanParameters.format);
        previewPanelUpdater.InitProgress(std::string(), 0, m_ScanParameters.bytes_per_line * m_ScanParameters.lines);

        m_ScanCallbackId = gtk_widget_add_tick_callback(
                m_MainWindow,
                [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                    auto *localApp = static_cast<App *>(data);
                    localApp->UpdatePreview();
                    return G_SOURCE_CONTINUE;
                },
                this, nullptr);
    }
}

void Gorfector::App::UpdatePreview()
{
    if (m_PreviewPanel == nullptr)
    {
        return;
    }

    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    SANE_Int readLength = 0;
    SANE_Byte *readBuffer = nullptr;
    int maxReadLength = 0;
    auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
    previewPanelUpdater.GetReadBuffer(readBuffer, maxReadLength);

    if (maxReadLength == 0 || !device->Read(readBuffer, maxReadLength, &readLength))
    {
        StopPreview();
        previewPanelUpdater.SetProgressCompleted();
        return;
    }

    previewPanelUpdater.CommitReadBuffer(readLength);
    previewPanelUpdater.IncreaseProgress(readLength);
}

void Gorfector::App::OnCancelClicked(GtkWidget *)
{
    if (m_AppState->IsScanning())
    {
        StopScan(false);
    }
    else if (m_AppState->IsPreviewing())
    {
        StopPreview();
    }
}

void IncrementPath(std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
        return;

    auto extension = path.extension();
    auto filename = path.filename().replace_extension().string();
    auto directory = path.parent_path();

    int counter;
    std::string fileNameFormat;

    auto counterRegex = std::regex("(.+)([.-_])([0-9]+)$");
    std::smatch match;
    if (std::regex_match(filename, match, counterRegex) && match.size() == 4)
    {
        auto baseName = match[1].str();
        auto separator = match[2].str();
        counter = std::stoi(match[3].str());
        fileNameFormat =
                baseName + separator + "{:0" + std::to_string(match[3].str().length()) + "d}" + extension.string();
    }
    else
    {
        counter = 1;
        fileNameFormat = filename + "_{:02d}" + extension.string();
    }

    auto newFilePath = directory / std::vformat(fileNameFormat, std::make_format_args(counter));
    while (std::filesystem::exists(newFilePath))
    {
        counter++;
        newFilePath = directory / std::vformat(fileNameFormat, std::make_format_args(counter));
    }
    path = newFilePath;
}

bool Gorfector::App::CheckFileOutputOptions(const OutputOptionsState *scanOptions) const
{
    const auto &dirPath = scanOptions->GetOutputDirectory();
    const auto &fileName = scanOptions->GetOutputFileName();

    if (dirPath.empty())
    {
        m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
        ZooLib::ShowUserError(
                ADW_APPLICATION_WINDOW(m_MainWindow), _("Select a directory where to save the image files."));
        return false;
    }

    if (fileName.empty())
    {
        m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Enter a file name to save the image files."));
        return false;
    }

    if (SelectFileWriter(fileName) == nullptr)
    {
        return false;
    }

    return true;
}

void Gorfector::App::ScanToFile(const OutputOptionsState *scanOptions)
{
    const auto &dirPath = scanOptions->GetOutputDirectory();
    const auto &fileName = scanOptions->GetOutputFileName();

    if (!std::filesystem::exists(dirPath) && scanOptions->GetCreateMissingDirectories())
    {
        std::filesystem::create_directory(dirPath);
    }

    if (!std::filesystem::exists(dirPath))
    {
        m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
        auto dirPathStr = dirPath.string();
        ZooLib::ShowUserError(
                ADW_APPLICATION_WINDOW(m_MainWindow),
                std::vformat(_("Directory does not exists: {}."), std::make_format_args(dirPathStr)));
        return;
    }

    m_ImageFilePath = dirPath / fileName;
    if (std::filesystem::exists(m_ImageFilePath))
    {
        switch (scanOptions->GetFileExistsAction())
        {
            case OutputOptionsState::FileExistsAction::e_Cancel:
            {
                m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
                auto imageFilePathStr = m_ImageFilePath.string();
                ZooLib::ShowUserError(
                        ADW_APPLICATION_WINDOW(m_MainWindow),
                        std::vformat(_("File '{}' already exists."), std::make_format_args(imageFilePathStr)));
                return;
            }
            case OutputOptionsState::FileExistsAction::e_IncrementCounter:
            {
                IncrementPath(m_ImageFilePath);
                if (std::filesystem::exists(m_ImageFilePath))
                {
                    m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
                    auto imageFilePathStr = m_ImageFilePath.string();
                    ZooLib::ShowUserError(
                            ADW_APPLICATION_WINDOW(m_MainWindow),
                            std::vformat(
                                    _("Incrementing file name '{}' failed to produce a unique name."),
                                    std::make_format_args(imageFilePathStr)));
                    return;
                }

                m_FileWriter = SelectFileWriter(m_ImageFilePath);
                if (m_FileWriter != nullptr)
                {
                    StartScan();
                }
                break;
            }
            case OutputOptionsState::FileExistsAction::e_Overwrite:
            {
                m_FileWriter = SelectFileWriter(m_ImageFilePath);
                if (m_FileWriter != nullptr)
                {
                    auto alert = adw_alert_dialog_new(_("File already exists"), nullptr);
                    adw_alert_dialog_format_body(
                            ADW_ALERT_DIALOG(alert), _("The file \"%s\" already exists. Really overwrite it?"),
                            m_ImageFilePath.c_str());
                    adw_alert_dialog_add_responses(
                            ADW_ALERT_DIALOG(alert), "cancel", _("Cancel"), "overwrite", _("Overwrite"), NULL);
                    adw_alert_dialog_set_response_appearance(
                            ADW_ALERT_DIALOG(alert), "overwrite", ADW_RESPONSE_DESTRUCTIVE);
                    adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(alert), "cancel");
                    adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(alert), "cancel");
                    ZooLib::ConnectGtkSignal(this, &App::OnOverwriteAlertResponse, alert, "response");
                    adw_dialog_present(alert, GTK_WIDGET(GetMainWindow()));
                }
                break;
            }
        }
    }
    else
    {
        m_FileWriter = SelectFileWriter(m_ImageFilePath);
        if (m_FileWriter != nullptr)
        {
            StartScan();
        }
    }
}

void Gorfector::App::OnOverwriteAlertResponse(AdwAlertDialog *alert, gchar *response)
{
    if (strcmp(response, "overwrite") == 0)
    {
        StartScan();
    }
}

Gorfector::FileWriter *Gorfector::App::SelectFileWriter(const std::string &path) const
{
    auto fileWriter = FileWriter::GetFormatForPath(path);
    if (fileWriter == nullptr)
    {
        auto extensionsStr = std::string();
        for (auto format: FileWriter::GetFormats())
        {
            auto mainExt = format->GetExtensions()[0];
            extensionsStr += extensionsStr.empty() ? mainExt : ", " + mainExt;
        }

        m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
        auto imageFilePathStr = m_ImageFilePath.string();
        ZooLib::ShowUserError(
                ADW_APPLICATION_WINDOW(m_MainWindow),
                std::vformat(
                        _("The file name must ends with an extension that specify the image format, like {}"),
                        std::make_format_args(extensionsStr)));
    }

    return fileWriter;
}

void Gorfector::App::OnAddToScanListClicked(GtkWidget *)
{
    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    auto scanOptions = m_ScanOptionsPanel->GetDeviceOptionsState();
    auto outputOptions = m_ScanOptionsPanel->GetOutputOptionsState();

    auto destination = outputOptions->GetOutputDestination();
    if (destination == OutputOptionsState::OutputDestination::e_File)
    {
        if (!CheckFileOutputOptions(outputOptions))
        {
            return;
        }
    }

    m_Dispatcher.Dispatch(CreateScanListItemCommand(scanOptions, outputOptions));
}

void Gorfector::App::OnScanClicked(GtkWidget *)
{
    if (m_ScanOptionsPanel == nullptr)
    {
        return;
    }

    auto outputOptions = m_ScanOptionsPanel->GetOutputOptionsState();
    auto destination = outputOptions->GetOutputDestination();
    if (destination == OutputOptionsState::OutputDestination::e_File)
    {
        if (CheckFileOutputOptions(outputOptions))
        {
            ScanToFile(outputOptions);
        }
    }
    else if (destination == OutputOptionsState::OutputDestination::e_Email)
    {
        m_ImageFilePath = GetTemporaryDirectory() / "email_image_0000.jpg";
        IncrementPath(m_ImageFilePath);
        m_FileWriter = FileWriter::GetFormatByType<JpegWriter>();
        StartScan();
    }
    else if (destination == OutputOptionsState::OutputDestination::e_Printer)
    {
        m_ImageFilePath = GetTemporaryDirectory() / "email_image_0000.tif";
        IncrementPath(m_ImageFilePath);
        m_FileWriter = FileWriter::GetFormatByType<TiffWriter>();
        StartScan();
    }
}

void Gorfector::App::StartScan()
{
    const auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    try
    {
        device->StartScan();
        auto updater = AppState::Updater(m_AppState);
        updater.SetIsScanning(true);
    }
    catch (const std::runtime_error &e)
    {
        StopScan(false);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), e.what());
        return;
    }

    // Must be called after StartScan()
    device->GetParameters(&m_ScanParameters);

    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        StopScan(false);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
        return;
    }

    if (m_ScanParameters.depth != 1 && m_ScanParameters.depth != 8 && m_ScanParameters.depth != 16)
    {
        StopScan(false);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported depth"));
        return;
    }

    if (auto error = m_FileWriter->CreateFile(*this, m_ImageFilePath, GetDeviceOptions(), m_ScanParameters, nullptr);
        error != FileWriter::Error::None)
    {
        StopScan(false);
        auto errorString = std::string(_("Failed to create file: ")) + m_FileWriter->GetError(error);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), errorString);
        return;
    }

    auto linesIn1MB = 1024 * 1024 / m_ScanParameters.bytes_per_line;
    m_BufferSize = m_ScanParameters.bytes_per_line * linesIn1MB;
    m_Buffer = static_cast<SANE_Byte *>(calloc(m_BufferSize, sizeof(SANE_Byte)));
    m_WriteOffset = 0;

    auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
    previewPanelUpdater.InitProgress(
            m_ImageFilePath.filename(), 0, m_ScanParameters.bytes_per_line * m_ScanParameters.lines);

    m_ScanCallbackId = gtk_widget_add_tick_callback(
            GTK_WIDGET(m_MainWindow),
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *localApp = static_cast<App *>(data);
                localApp->UpdateScan();
                return G_SOURCE_CONTINUE;
            },
            this, nullptr);
}

void Gorfector::App::UpdateScan()
{
    // TODO: UpdateScan should not be called as a tick callback (not frequent enough)
    // TODO: Saving to file should be done in a separate thread

    if (m_Buffer == nullptr || m_BufferSize == 0)
    {
        return;
    }

    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    if (m_FileWriter == nullptr)
    {
        device->CancelScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported file format"));
        return;
    }

    auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());

    SANE_Int readLength = 0;
    auto requestedLength = m_BufferSize - m_WriteOffset;
    if (requestedLength > 0)
    {
        if (!device->Read(m_Buffer + m_WriteOffset, requestedLength, &readLength))
        {
            previewPanelUpdater.SetProgressCompleted();

            auto dataEnd = m_WriteOffset + readLength;
            auto availableLines = dataEnd / m_ScanParameters.bytes_per_line;
            m_FileWriter->AppendBytes(m_Buffer, availableLines, m_ScanParameters);

            StopScan(true);

            return;
        }
    }

    auto availableBytes = m_WriteOffset + readLength;
    auto availableLines = availableBytes / m_ScanParameters.bytes_per_line;
    auto savedBytes = m_FileWriter->AppendBytes(m_Buffer, availableLines, m_ScanParameters);
    if (savedBytes < availableBytes)
    {
        memmove(m_Buffer, m_Buffer + savedBytes, availableBytes - savedBytes);
        m_WriteOffset = availableBytes - savedBytes;
    }
    else
    {
        m_WriteOffset = 0;
    }

    previewPanelUpdater.IncreaseProgress(readLength);
}

void Gorfector::App::StopPreview()
{
    auto device = GetDevice();
    if (device != nullptr)
    {
        device->CancelScan();
    }

    if (m_ScanCallbackId != 0)
    {
        gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), m_ScanCallbackId);
        m_ScanCallbackId = 0;
    }

    RestoreOptionsAfterPreview();

    auto updater = AppState::Updater(m_AppState);
    updater.SetIsPreviewing(false);
}

void Gorfector::App::StopScan(bool completed)
{
    auto device = GetDevice();
    if (device != nullptr)
    {
        device->CancelScan();
    }

    if (m_ScanCallbackId != 0)
    {
        gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), m_ScanCallbackId);
        m_ScanCallbackId = 0;
    }

    if (m_FileWriter != nullptr)
    {
        m_FileWriter->CloseFile();
        m_FileWriter = nullptr;
    }

    free(m_Buffer);
    m_Buffer = nullptr;
    m_BufferSize = 0;
    m_WriteOffset = 0;

    if (completed && std::filesystem::exists(m_ImageFilePath))
    {
        auto scanOptions = m_ScanOptionsPanel->GetOutputOptionsState();
        auto destination = scanOptions->GetOutputDestination();
        if (destination == OutputOptionsState::OutputDestination::e_Email)
        {
            auto command = "xdg-email --attach " + m_ImageFilePath.string();
            std::system(command.c_str());
        }
        else if (destination == OutputOptionsState::OutputDestination::e_Printer)
        {
            auto printDialog = gtk_print_dialog_new();
            auto imageFile = g_file_new_for_path(m_ImageFilePath.c_str());
            gtk_print_dialog_print_file(
                    printDialog, GTK_WINDOW(m_MainWindow), nullptr, imageFile, nullptr, nullptr, nullptr);
        }
    }

    auto updater = AppState::Updater(m_AppState);
    updater.SetIsScanning(false);
}
