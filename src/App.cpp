#include <adwaita.h>
#include <format>
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
#include "PathUtils.hpp"
#include "PreferencesView.hpp"
#include "PresetPanel.hpp"
#include "PreviewPanel.hpp"
#include "PreviewScanProcess.hpp"
#include "ScanListPanel.hpp"
#include "ScanOptionsPanel.hpp"
#include "SingleScanProcess.hpp"
#include "Writers/FileWriter.hpp"
#include "Writers/JpegWriter.hpp"
#include "Writers/PngWriter.hpp"
#include "Writers/TiffWriter.hpp"
#include "ZooLib/AppMenuBarBuilder.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/SignalSupport.hpp"

Gorfector::App::App(int argc, char **argv, bool testMode)
    : Application(testMode)
{
    SANE_Int saneVersion;
    if (SANE_STATUS_GOOD != sane_init(&saneVersion, nullptr))
    {
        sane_exit();
        throw std::runtime_error("Failed to initialize SANE");
    }

    auto prefDir = GetUserConfigDirectoryPath();
    auto prefFilePath = prefDir / "preferences.json";
    m_State.SetPreferencesFilePath(prefFilePath);

    bool devMode{};
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--dev") == 0)
        {
            devMode = true;
        }
    }

    m_AppState = new AppState(&m_State, devMode);
    m_State.LoadFromPreferencesFile(m_AppState);

    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_ViewUpdateObserver);

    m_DeviceSelectorState = new DeviceSelectorState(&m_State);
    m_DeviceSelectorObserver = new DeviceSelectorObserver(m_DeviceSelectorState, m_AppState);
    m_ObserverManager.AddObserver(m_DeviceSelectorObserver);

    FileWriter::Register<TiffWriter>(&m_State, App::GetApplicationName());
    FileWriter::Register<JpegWriter>(&m_State, App::GetApplicationName());
    FileWriter::Register<PngWriter>(&m_State, App::GetApplicationName());
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
    std::string cssPath = std::string("/com/patrickfournier/gorfector/css/gorfector.css");
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

    auto bottomAlignedSection = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(bottomAlignedSection, true);
    gtk_widget_set_vexpand(bottomAlignedSection, true);
    gtk_widget_set_valign(bottomAlignedSection, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(m_SettingsBox), bottomAlignedSection);

    m_PresetPanel = PresetPanel::Create(&m_Dispatcher, this);
    auto presetsBox = m_PresetPanel->GetRootWidget();
    gtk_widget_set_margin_bottom(presetsBox, 0);
    gtk_widget_set_margin_top(presetsBox, 0);
    gtk_widget_set_margin_start(presetsBox, 0);
    gtk_widget_set_margin_end(presetsBox, 0);
    gtk_box_append(GTK_BOX(bottomAlignedSection), presetsBox);
    gtk_widget_set_hexpand(presetsBox, true);

    m_PreviewPanel = PreviewPanel::Create(&m_Dispatcher, this);
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

    m_ScanListPanel = ScanListPanel::Create(&m_Dispatcher, this);
    auto scanListBox = m_ScanListPanel->GetRootWidget();
    adw_clamp_set_child(ADW_CLAMP(clamp), scanListBox);
}

void Gorfector::App::RemoveScanListUI()
{
    m_RightPaned = nullptr;
    m_ScanListPanel = nullptr;
}

void Gorfector::App::PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder)
{
    menuBarBuilder->BeginSection()
            ->AddMenuItem(_("Select Device..."), "app.select_device")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem(_("Use Scan List"), "app.scanlist")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem(_("Settings..."), "app.preferences")
            ->AddMenuItem(_("Help..."), "app.help")
            ->AddMenuItem(_("About..."), "app.about")
            ->EndSection();

    BindMethodToAction<App>("select_device", &App::ShowSelectDeviceDialog, this);
    BindMethodToAction<Application>("quit", &Application::Quit, this);
    BindCommandToToggleAction<ToggleUseScanList>("scanlist", m_AppState->GetUseScanList(), m_AppState);
    BindMethodToAction<App>("preferences", &App::ShowPreferenceDialog, this);
    BindMethodToAction<App>("help", &App::ShowHelp, this);
    BindMethodToAction<App>("about", &App::ShowAboutDialog, this);

    SetAcceleratorForAction("app.quit", {"<Ctrl>Q"});
    SetAcceleratorForAction("app.undo", {"<Ctrl>Z"});
    SetAcceleratorForAction("app.redo", {"<Ctrl><Shift>Z"});
}

void Gorfector::App::ShowSelectDeviceDialog(GSimpleAction *action, GVariant *parameter)
{
    auto deviceSelector = DeviceSelector::Create(&m_Dispatcher, this, m_DeviceSelectorState);
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

void Gorfector::App::ShowPreferenceDialog(GSimpleAction *action, GVariant *parameter)
{
    auto dialog = adw_preferences_dialog_new();
    auto preferencePages = PreferencesView::Create(
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

void Gorfector::App::ShowHelp(GSimpleAction *action, GVariant *parameter)
{
    auto launcher = gtk_uri_launcher_new("help:gorfector");
    gtk_uri_launcher_launch(launcher, GTK_WINDOW(m_MainWindow), nullptr, nullptr, nullptr);
}

void Gorfector::App::ShowAboutDialog(GSimpleAction *action, GVariant *parameter)
{
    auto dialogWindow = adw_about_dialog_new();
    adw_about_dialog_set_application_icon(ADW_ABOUT_DIALOG(dialogWindow), GetApplicationId().c_str());
    adw_about_dialog_set_application_name(ADW_ABOUT_DIALOG(dialogWindow), k_ApplicationName);
    adw_about_dialog_set_version(ADW_ABOUT_DIALOG(dialogWindow), "0.1");
    adw_about_dialog_set_comments(ADW_ABOUT_DIALOG(dialogWindow), _("An application to scan images."));
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
        m_ScanOptionsPanel =
                ScanOptionsPanel::Create(GetSelectorSaneInitId(), GetSelectorDeviceName(), &m_Dispatcher, this);
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
        }
        else if (!GetSelectorDeviceName().empty())
        {
            gtk_widget_set_sensitive(m_PreviewButton, true);
            gtk_widget_set_sensitive(m_ScanButton, true);
            gtk_widget_set_sensitive(m_CancelButton, false);
        }
        else
        {
            gtk_widget_set_sensitive(m_PreviewButton, false);
            gtk_widget_set_sensitive(m_ScanButton, false);
            gtk_widget_set_sensitive(m_CancelButton, false);
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

void Gorfector::App::OnPreviewClicked(GtkWidget *)
{
    auto *finishCallback = new std::function<void()>([this]() { this->m_ScanProcess = nullptr; });

    m_ScanProcess = new PreviewScanProcess(
            GetDevice(), m_PreviewPanel->GetState(), m_AppState, GetDeviceOptions(), GetOutputOptions(), m_MainWindow,
            finishCallback);

    if (!m_ScanProcess->Start())
    {
        // finishCallback is deleted in the destructor of ScanProcess
        delete m_ScanProcess;
        m_ScanProcess = nullptr;
    }
}

void Gorfector::App::OnCancelClicked(GtkWidget *)
{
    if (m_ScanProcess != nullptr)
    {
        m_ScanProcess->Cancel();
    }
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

    auto imageFilePath = dirPath / fileName;
    if (std::filesystem::exists(imageFilePath))
    {
        switch (scanOptions->GetFileExistsAction())
        {
            case OutputOptionsState::FileExistsAction::e_Cancel:
            {
                m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
                auto imageFilePathStr = imageFilePath.string();
                ZooLib::ShowUserError(
                        ADW_APPLICATION_WINDOW(m_MainWindow),
                        std::vformat(_("File '{}' already exists."), std::make_format_args(imageFilePathStr)));
                return;
            }
            case OutputOptionsState::FileExistsAction::e_IncrementCounter:
            {
                IncrementPath(imageFilePath);
                if (std::filesystem::exists(imageFilePath))
                {
                    m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
                    auto imageFilePathStr = imageFilePath.string();
                    ZooLib::ShowUserError(
                            ADW_APPLICATION_WINDOW(m_MainWindow),
                            std::vformat(
                                    _("Incrementing file name '{}' failed to produce a unique name."),
                                    std::make_format_args(imageFilePathStr)));
                    return;
                }

                auto fileWriter = SelectFileWriter(imageFilePath);
                if (fileWriter != nullptr)
                {
                    StartScan(fileWriter, imageFilePath);
                }
                break;
            }
            case OutputOptionsState::FileExistsAction::e_Overwrite:
            {
                auto fileWriter = SelectFileWriter(imageFilePath);
                if (fileWriter != nullptr)
                {
                    auto alert = adw_alert_dialog_new(_("File already exists"), nullptr);
                    adw_alert_dialog_format_body(
                            ADW_ALERT_DIALOG(alert), _("The file \"%s\" already exists. Really overwrite it?"),
                            imageFilePath.c_str());
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
        auto fileWriter = SelectFileWriter(imageFilePath);
        if (fileWriter != nullptr)
        {
            StartScan(fileWriter, imageFilePath);
        }
    }
}

void Gorfector::App::OnOverwriteAlertResponse(AdwAlertDialog *alert, gchar *response)
{
    if (strcmp(response, "overwrite") == 0)
    {
        auto outputOptions = m_ScanOptionsPanel->GetOutputOptionsState();
        auto imageFilePath = outputOptions->GetOutputDirectory() / outputOptions->GetOutputFileName();
        auto fileWriter = SelectFileWriter(imageFilePath);
        StartScan(fileWriter, imageFilePath);
    }
}

Gorfector::FileWriter *Gorfector::App::SelectFileWriter(const std::string &path) const
{
    auto fileWriter = FileWriter::GetFileWriterForPath(path);
    if (fileWriter == nullptr)
    {
        auto extensionsStr = std::string();
        for (auto format: FileWriter::GetWriters())
        {
            auto mainExt = format->GetExtensions()[0];
            extensionsStr += extensionsStr.empty() ? mainExt : ", " + mainExt;
        }

        m_ScanOptionsPanel->SelectPage(ScanOptionsPanel::Page::e_FileOutput);
        ZooLib::ShowUserError(
                ADW_APPLICATION_WINDOW(m_MainWindow),
                std::vformat(
                        _("The file name must ends with an extension that specify the image format, like {}"),
                        std::make_format_args(extensionsStr)));
    }

    return fileWriter;
}

void Gorfector::App::OnScanClicked(GtkWidget *)
{
    if (m_ScanOptionsPanel == nullptr)
    {
        return;
    }

    if (m_ScanProcess != nullptr)
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
        auto imageFilePath = GetTemporaryDirectory() / "email_image_0000.jpg";
        IncrementPath(imageFilePath);
        auto fileWriter = FileWriter::GetFormatByType<JpegWriter>();
        StartScan(fileWriter, imageFilePath);
    }
    else if (destination == OutputOptionsState::OutputDestination::e_Printer)
    {
        auto imageFilePath = GetTemporaryDirectory() / "email_image_0000.tif";
        IncrementPath(imageFilePath);
        auto fileWriter = FileWriter::GetFormatByType<TiffWriter>();
        StartScan(fileWriter, imageFilePath);
    }
}

void Gorfector::App::StartScan(FileWriter *fileWriter, const std::filesystem::path &imageFilePath)
{
    auto *finishCallback = new std::function<void()>([this]() { this->m_ScanProcess = nullptr; });

    m_ScanProcess = new SingleScanProcess(
            GetDevice(), m_PreviewPanel->GetState(), m_AppState, GetDeviceOptions(), GetOutputOptions(), m_MainWindow,
            fileWriter, imageFilePath, finishCallback);
    if (!m_ScanProcess->Start())
    {
        // finishCallback is deleted in the destructor of ScanProcess
        delete m_ScanProcess;
        m_ScanProcess = nullptr;
    }
}
