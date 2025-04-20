#include <adwaita.h>
#include <memory>
#include <sane/sane.h>
#include <stdexcept>

#include "App.hpp"

#include "AppState.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "Commands/SetScanMode.hpp"
#include "DeviceOptionsObserver.hpp"
#include "DeviceOptionsPanel.hpp"
#include "DeviceSelector.hpp"
#include "DeviceSelectorObserver.hpp"
#include "Gettext.hpp"
#include "PreferencesView.hpp"
#include "PreviewPanel.hpp"
#include "Writers/FileWriter.hpp"
#include "Writers/JpegWriter.hpp"
#include "Writers/PngWriter.hpp"
#include "Writers/TiffWriter.hpp"
#include "ZooLib/AppMenuBarBuilder.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/SignalSupport.hpp"

ZooScan::App::App(int argc, char **argv)
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
    m_State.SetFilePath(prefFilePath);

    bool devMode{};
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--dev") == 0)
        {
            devMode = true;
        }
    }

    m_AppState = new AppState(&m_State, devMode);

    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_ViewUpdateObserver);

    m_DeviceSelectorState = new DeviceSelectorState(&m_State);
    m_DeviceSelectorObserver = new DeviceSelectorObserver(m_DeviceSelectorState, m_AppState);
    m_ObserverManager.AddObserver(m_DeviceSelectorObserver);

    FileWriter::Register<TiffWriter>(&m_State);
    FileWriter::Register<JpegWriter>(&m_State);
    FileWriter::Register<PngWriter>(&m_State);
}

ZooScan::App::~App()
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

void ZooScan::App::OnActivate(GtkApplication *app)
{
    Application::OnActivate(app);

    if (GetSelectorDeviceName().empty())
    {
        auto action = g_action_map_lookup_action(G_ACTION_MAP(app), "select_device");
        g_action_activate(action, nullptr);
    }
}

GtkWidget *ZooScan::App::CreateContent()
{
    auto *display = gdk_display_get_default();
    auto *cssProvider = gtk_css_provider_new();
    std::string cssPath = std::string("/com/patrickfournier/zooscan/resources/zooscan.css");
    gtk_css_provider_load_from_resource(cssProvider, cssPath.c_str());
    gtk_style_context_add_provider_for_display(
            display, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
    g_object_unref(cssProvider);

    auto deviceSelected = !GetSelectorDeviceName().empty();

    auto *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_bottom(paned, 15);
    gtk_widget_set_margin_top(paned, 0);
    gtk_widget_set_margin_start(paned, 10);
    gtk_widget_set_margin_end(paned, 10);

    gtk_paned_set_wide_handle(GTK_PANED(paned), true);
    gtk_paned_set_position(GTK_PANED(paned), 200);

    auto clamp = adw_clamp_new();
    adw_clamp_set_unit(ADW_CLAMP(clamp), ADW_LENGTH_UNIT_SP);
    adw_clamp_set_tightening_threshold(ADW_CLAMP(clamp), 600);
    adw_clamp_set_maximum_size(ADW_CLAMP(clamp), 800);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(clamp), GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned), clamp);

    m_SettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_bottom(m_SettingsBox, 0);
    gtk_widget_set_margin_top(m_SettingsBox, 0);
    gtk_widget_set_margin_start(m_SettingsBox, 0);
    gtk_widget_set_margin_end(m_SettingsBox, 10);
    adw_clamp_set_child(ADW_CLAMP(clamp), m_SettingsBox);

    m_PreviewButton = gtk_button_new_with_label(_("Preview"));
    ConnectGtkSignal(this, &App::OnPreviewClicked, m_PreviewButton, "clicked");
    gtk_widget_set_sensitive(m_PreviewButton, deviceSelected);
    gtk_box_append(GTK_BOX(m_SettingsBox), m_PreviewButton);

    m_ScanButton = gtk_button_new_with_label(_("Scan"));
    ConnectGtkSignal(this, &App::OnScanClicked, m_ScanButton, "clicked");
    gtk_widget_set_sensitive(m_ScanButton, deviceSelected);
    gtk_box_append(GTK_BOX(m_SettingsBox), m_ScanButton);

    m_CancelButton = gtk_button_new_with_label(_("Cancel Scan"));
    ConnectGtkSignal(this, &App::OnCancelClicked, m_CancelButton, "clicked");
    gtk_widget_set_sensitive(m_CancelButton, false);
    gtk_box_append(GTK_BOX(m_SettingsBox), m_CancelButton);

    m_PreviewPanel = ZooLib::View::Create<PreviewPanel>(&m_Dispatcher, this);
    auto previewBox = m_PreviewPanel->GetRootWidget();
    gtk_widget_set_margin_bottom(previewBox, 0);
    gtk_widget_set_margin_top(previewBox, 0);
    gtk_widget_set_margin_start(previewBox, 10);
    gtk_widget_set_margin_end(previewBox, 0);
    gtk_paned_set_end_child(GTK_PANED(paned), previewBox);

    return paned;
}

void ZooScan::App::PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder)
{
    menuBarBuilder->BeginSection()
            ->AddMenuItem(_("Select Device..."), "app.select_device")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem("Single Scan", "app.single")
            ->AddMenuItem("Multiple Scans", "app.multiple")
            ->EndSection()
            ->BeginSection()
            ->AddMenuItem(_("Settings..."), "app.preferences")
            ->AddMenuItem(_("About..."), "app.about")
            ->EndSection();

    BindMethodToAction<App>("select_device", &App::SelectDeviceDialog, this);
    BindMethodToAction<Application>("quit", &Application::Quit, this);
    BindCommandToToggleAction<SetSingleScanMode>("single", m_AppState->GetAppMode() == AppState::Single, m_AppState);
    BindCommandToToggleAction<SetBatchScanMode>("multiple", m_AppState->GetAppMode() == AppState::Batch, m_AppState);
    BindMethodToAction<App>("preferences", &App::PreferenceDialog, this);
    BindMethodToAction<App>("about", &App::AboutDialog, this);

    SetAcceleratorForAction("app.quit", {"<Ctrl>Q"});
    SetAcceleratorForAction("app.undo", {"<Ctrl>Z"});
    SetAcceleratorForAction("app.redo", {"<Ctrl><Shift>Z"});
}

void ZooScan::App::SelectDeviceDialog(GSimpleAction *action, GVariant *parameter)
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

void ZooScan::App::PreferenceDialog(GSimpleAction *action, GVariant *parameter)
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

void ZooScan::App::AboutDialog(GSimpleAction *action, GVariant *parameter)
{
    auto dialogWindow = adw_about_dialog_new();
    adw_about_dialog_set_application_icon(
            ADW_ABOUT_DIALOG(dialogWindow),
            // FIXME should use the icon in resources, but does not work
            "scanner-symbolic");
    adw_about_dialog_set_application_name(ADW_ABOUT_DIALOG(dialogWindow), "ZooScan");
    adw_about_dialog_set_version(ADW_ABOUT_DIALOG(dialogWindow), "0.1");
    adw_about_dialog_set_comments(
            ADW_ABOUT_DIALOG(dialogWindow), _("An application to scan images.")); // FIXME: could be more detailed
    adw_about_dialog_set_license_type(ADW_ABOUT_DIALOG(dialogWindow), GTK_LICENSE_GPL_3_0);
    adw_about_dialog_set_developer_name(ADW_ABOUT_DIALOG(dialogWindow), "Patrick Fournier");

    adw_dialog_present(ADW_DIALOG(dialogWindow), m_MainWindow);
}

const std::string &ZooScan::App::GetSelectorDeviceName() const
{
    if (m_DeviceSelectorState == nullptr)
    {
        return DeviceSelectorState::k_NullDeviceName;
    }

    return m_DeviceSelectorState->GetSelectedDeviceName();
}

int ZooScan::App::GetSelectorSaneInitId() const
{
    if (m_DeviceSelectorState == nullptr)
    {
        return 0;
    }

    return m_DeviceSelectorState->GetSelectorSaneInitId();
}

void ZooScan::App::Update(const std::vector<uint64_t> &lastSeenVersions)
{
    if (m_DeviceOptionsPanel != nullptr && (m_DeviceOptionsPanel->GetDeviceName() != GetSelectorDeviceName() ||
                                            m_DeviceOptionsPanel->GetSaneInitId() != GetSelectorSaneInitId()))
    {
        if (m_DeviceOptionsObserver != nullptr)
        {
            m_ObserverManager.RemoveObserver(m_DeviceOptionsObserver);
            delete m_DeviceOptionsObserver;
        }

        gtk_box_remove(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->GetRootWidget());
        // m_DeviceOptionsPanel is automatically deleted when its root widget is destroyed.
        m_DeviceOptionsPanel = nullptr;
        m_Dispatcher.UnregisterHandler<SetScanAreaCommand>();
    }

    if (m_DeviceOptionsPanel == nullptr && m_SettingsBox != nullptr && !GetSelectorDeviceName().empty())
    {
        m_DeviceOptionsPanel = ZooLib::View::Create<DeviceOptionsPanel>(
                GetSelectorSaneInitId(), GetSelectorDeviceName(), &m_Dispatcher, this);
        gtk_box_prepend(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->GetRootWidget());
        m_Dispatcher.RegisterHandler<SetScanAreaCommand, DeviceOptionsState>(
                SetScanAreaCommand::Execute, m_DeviceOptionsPanel->GetState());

        if (m_PreviewPanel != nullptr)
        {
            m_DeviceOptionsObserver =
                    new DeviceOptionsObserver(m_DeviceOptionsPanel->GetState(), m_PreviewPanel->GetState());
            m_ObserverManager.AddObserver(m_DeviceOptionsObserver);
        }
    }

    // Update the action associated with the scan mode
    auto action = g_action_map_lookup_action(G_ACTION_MAP(m_GtkApp), "single");
    g_action_change_state(action, g_variant_new_boolean(m_AppState->GetAppMode() == AppState::Single));
    action = g_action_map_lookup_action(G_ACTION_MAP(m_GtkApp), "multiple");
    g_action_change_state(action, g_variant_new_boolean(m_AppState->GetAppMode() == AppState::Batch));

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

const ZooScan::DeviceOptionsState *ZooScan::App::GetDeviceOptions() const
{
    if (m_DeviceOptionsPanel == nullptr)
        return nullptr;

    return m_DeviceOptionsPanel->GetState();
}

void ZooScan::App::RestoreScanOptions() const
{
    auto device = GetDevice();
    const auto options = GetDeviceOptions();
    if (device == nullptr || options == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_FALSE;
    device->SetOptionValue(options->PreviewIndex(), &isPreview, nullptr);

    if (options->ModeIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<std::string>(options->ModeIndex());
        std::string value = option->GetValue();
        std::unique_ptr<char[]> valueCString(new char[option->GetValueSize() + 1]);
        strcpy(valueCString.get(), value.c_str());
        device->SetOptionValue(options->ModeIndex(), valueCString.get(), nullptr);
    }

    if (options->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->BitDepthIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->BitDepthIndex(), &value, nullptr);
    }

    if (options->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = options->GetOption<int>(options->ResolutionIndex());
        int value = option->GetValue();
        device->SetOptionValue(options->ResolutionIndex(), &value, nullptr);
    }
}

int ZooScan::App::GetScanHeight() const
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

void ZooScan::App::OnPreviewClicked(GtkWidget *)
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

    if (options->ModeIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto settingDescription = device->GetOptionDescriptor(options->ModeIndex());
        auto i = 0;
        while (settingDescription->constraint.string_list[i] != nullptr &&
               !strcasestr(settingDescription->constraint.string_list[i], "color"))
        {
            i++;
        }
        if (settingDescription->constraint.string_list[i] != nullptr)
        {
            std::string mode = settingDescription->constraint.string_list[i];
            std::unique_ptr<char[]> modeCString(new char[mode.size() + 1]);
            strcpy(modeCString.get(), mode.c_str());
            device->SetOptionValue(options->ModeIndex(), modeCString.get(), nullptr);
        }
        else
        {
            device->SetOptionToDefault(options->ModeIndex());
        }
    }

    SANE_Int bitDepth = 8;
    if (options->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        device->SetOptionValue(options->BitDepthIndex(), &bitDepth, nullptr);
    }

    if (options->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto resolutionDescription = device->GetOptionDescriptor(options->ResolutionIndex());
        SANE_Int resolution;
        if (resolutionDescription->constraint_type == SANE_CONSTRAINT_RANGE)
        {
            resolution = resolutionDescription->constraint.range->min;
        }
        else if (resolutionDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
        {
            auto count = resolutionDescription->constraint.word_list[0];
            resolution = std::numeric_limits<int>::max();
            for (auto i = 0; i < count; i++)
            {
                resolution = std::min(resolution, resolutionDescription->constraint.word_list[i]);
            }
        }
        else
        {
            resolution = resolutionDescription->type == SANE_TYPE_FIXED ? SANE_FIX(300) : 300;
        }
        device->SetOptionValue(options->ResolutionIndex(), &resolution, nullptr);
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
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), e.what());
        return;
    }

    device->GetParameters(&m_ScanParameters);
    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
        return;
    }

    if (m_ScanParameters.depth != 8)
    {
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported depth"));
        return;
    }

    if (m_PreviewPanel != nullptr)
    {
        auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
        previewPanelUpdater.PrepareForScan(
                m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, m_ScanParameters.lines);
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

void ZooScan::App::UpdatePreview()
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
        StopScan();
        previewPanelUpdater.SetProgressCompleted();
        return;
    }

    previewPanelUpdater.CommitReadBuffer(readLength);
    previewPanelUpdater.IncreaseProgress(readLength);
}

void ZooScan::App::OnCancelClicked(GtkWidget *)
{
    StopScan();
}

void OnFileSave(GObject *saveDialog, GAsyncResult *res, gpointer data)
{
    auto app = static_cast<ZooScan::App *>(data);
    GError *error;
    auto file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(saveDialog), res, &error);
    app->OnFileSave(file);
}

void ZooScan::App::OnScanClicked(GtkWidget *)
{
    const auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    auto fileSaveDialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(GTK_FILE_DIALOG(fileSaveDialog), _("Save Scan"));
    gtk_file_dialog_set_initial_name(GTK_FILE_DIALOG(fileSaveDialog), FileWriter::DefaultFileName().c_str());

    GListStore *filters = g_list_store_new(G_TYPE_OBJECT);
    auto filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All Files"));
    gtk_file_filter_add_pattern(filter, "*");
    g_list_store_append(filters, filter);

    for (auto format: FileWriter::GetFormats())
    {
        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, format->GetName().c_str());

        for (const auto &ext: format->GetExtensions())
        {
            gtk_file_filter_add_pattern(filter, ("*" + ext).c_str());
        }
        g_list_store_append(filters, filter);
    }
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(GTK_FILE_DIALOG(fileSaveDialog), G_LIST_MODEL(filters));

    gtk_file_dialog_save(GTK_FILE_DIALOG(fileSaveDialog), GTK_WINDOW(m_MainWindow), nullptr, ::OnFileSave, this);
}

void ZooScan::App::OnFileSave(GFile *file)
{
    if (file == nullptr)
    {
        m_ImageFilePath = std::filesystem::path();
        return;
    }

    auto path = g_file_get_path(file);
    if (path == nullptr)
    {
        m_ImageFilePath = std::filesystem::path();
        return;
    }

    m_ImageFilePath = std::filesystem::path(path);
    g_free(path);

    m_FileWriter = FileWriter::GetFormatForPath(m_ImageFilePath);
    if (m_FileWriter == nullptr)
    {
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Cannot determine image file format"));
        return;
    }

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
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), e.what());
        return;
    }

    // Must be called after StartScan()
    device->GetParameters(&m_ScanParameters);

    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
        return;
    }

    if (m_ScanParameters.depth != 8 && m_ScanParameters.depth != 16)
    {
        StopScan();
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported depth"));
        return;
    }

    if (auto error = m_FileWriter->CreateFile(*this, m_ImageFilePath, GetDeviceOptions(), m_ScanParameters, nullptr);
        error != FileWriter::Error::None)
    {
        StopScan();
        auto errorString = std::string(_("Failed to create file: ")) + m_FileWriter->GetError(error);
        ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), errorString.c_str());
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

void ZooScan::App::UpdateScan()
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
            m_FileWriter->AppendBytes(
                    m_Buffer, availableLines, m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line,
                    m_ScanParameters.depth);

            StopScan();

            return;
        }
    }

    auto availableBytes = m_WriteOffset + readLength;
    auto availableLines = availableBytes / m_ScanParameters.bytes_per_line;
    auto savedBytes = m_FileWriter->AppendBytes(
            m_Buffer, availableLines, m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line,
            m_ScanParameters.depth);
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

void ZooScan::App::StopScan()
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

    if (m_AppState->IsPreviewing())
    {
        RestoreScanOptions();
    }

    if (m_AppState->IsScanning())
    {
        if (m_FileWriter != nullptr)
        {
            m_FileWriter->CloseFile();
        }

        free(m_Buffer);
        m_Buffer = nullptr;
        m_BufferSize = 0;
        m_WriteOffset = 0;
    }

    auto updater = AppState::Updater(m_AppState);
    updater.SetIsPreviewing(false);
    updater.SetIsScanning(false);
}
