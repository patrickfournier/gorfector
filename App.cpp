#include <memory>
#include <sane/sane.h>
#include <stdexcept>

#include "App.hpp"

#include "AppState.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "DeviceOptionsObserver.hpp"
#include "DeviceOptionsPanel.hpp"
#include "DeviceSelector.hpp"
#include "DeviceSelectorObserver.hpp"
#include "PreviewPanel.hpp"
#include "Writers/FileWriter.hpp"
#include "Writers/JpegWriter.hpp"
#include "Writers/PngWriter.hpp"
#include "Writers/TiffWriter.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/SignalSupport.hpp"


ZooScan::App::App()
{
    SANE_Int saneVersion;
    if (SANE_STATUS_GOOD != sane_init(&saneVersion, nullptr))
    {
        sane_exit();
        throw std::runtime_error("Failed to initialize SANE");
    }

    m_AppState = new AppState(&m_State);

    m_ViewUpdateObserver = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_ViewUpdateObserver);

    FileWriter::Register<TiffWriter>(&m_State);
    FileWriter::Register<JpegWriter>(&m_State);
    FileWriter::Register<PngWriter>(&m_State);
}

ZooScan::App::~App()
{
    m_Dispatcher.UnregisterHandler<SetScanAreaCommand>();

    m_ObserverManager.RemoveObserver(m_DeviceSelectorObserver);
    delete m_DeviceSelectorObserver;

    delete m_DeviceSelector;
    delete m_DeviceOptionsPanel;
    delete m_PreviewPanel;

    m_ObserverManager.RemoveObserver(m_ViewUpdateObserver);
    delete m_ViewUpdateObserver;

    delete m_AppState;

    if (m_Buffer != nullptr)
    {
        free(m_Buffer);
    }

    sane_exit();

    FileWriter::Clear();
}

void ZooScan::App::PopulateMainWindow()
{
    auto *display = gdk_display_get_default();
    auto *cssProvider = gtk_css_provider_new();
    std::string cssPath = std::string("/com/patrickfournier/zooscan/resources/zooscan.css");
    gtk_css_provider_load_from_resource(cssProvider, cssPath.c_str());
    gtk_style_context_add_provider_for_display(
            display, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
    g_object_unref(cssProvider);

    auto *grid = gtk_grid_new();
    gtk_window_set_child(m_MainWindow, grid);

    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);

    m_DeviceSelector = new DeviceSelector(&m_Dispatcher, this);
    gtk_grid_attach(GTK_GRID(grid), m_DeviceSelector->RootWidget(), 0, 0, 2, 1);

    m_DeviceSelectorObserver = new DeviceSelectorObserver(m_DeviceSelector->GetState(), m_AppState);
    m_ObserverManager.AddObserver(m_DeviceSelectorObserver);

    m_SettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_grid_attach(GTK_GRID(grid), m_SettingsBox, 0, 1, 1, 1);

    auto button = gtk_button_new_with_label("Preview");
    ConnectGtkSignal(this, &App::OnPreviewClicked, button, "clicked");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

    button = gtk_button_new_with_label("Scan");
    ConnectGtkSignal(this, &App::OnScanClicked, button, "clicked");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);

    m_PreviewPanel = new PreviewPanel(&m_Dispatcher, this);
    gtk_grid_attach(GTK_GRID(grid), m_PreviewPanel->GetRootWidget(), 1, 1, 1, 3);
}

const std::string &ZooScan::App::GetSelectorDeviceName() const
{
    if (m_DeviceSelector == nullptr)
    {
        return DeviceSelectorState::k_NullDeviceName;
    }

    return m_DeviceSelector->GetState()->GetSelectedDeviceName();
}

int ZooScan::App::GetSelectorSaneInitId() const
{
    if (m_DeviceSelector == nullptr)
    {
        return 0;
    }

    return m_DeviceSelector->GetState()->GetSelectorSaneInitId();
}

void ZooScan::App::Update(u_int64_t lastSeenVersion)
{
    if (m_IsScanning)
    {
        UpdateScan();
    }

    if (m_DeviceOptionsPanel != nullptr && (m_DeviceOptionsPanel->GetDeviceName() != GetSelectorDeviceName() ||
                                            m_DeviceOptionsPanel->GetSaneInitId() != GetSelectorSaneInitId()))
    {
        if (m_DeviceOptionsObserver != nullptr)
        {
            m_ObserverManager.RemoveObserver(m_DeviceOptionsObserver);
            delete m_DeviceOptionsObserver;
        }

        gtk_box_remove(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->GetRootWidget());
        delete m_DeviceOptionsPanel;
        m_DeviceOptionsPanel = nullptr;
        m_Dispatcher.UnregisterHandler<SetScanAreaCommand>();
    }

    if (m_DeviceOptionsPanel == nullptr && m_SettingsBox != nullptr && !GetSelectorDeviceName().empty())
    {
        m_DeviceOptionsPanel =
                new DeviceOptionsPanel(GetSelectorSaneInitId(), GetSelectorDeviceName(), &m_Dispatcher, this);
        gtk_box_append(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->GetRootWidget());
        m_Dispatcher.RegisterHandler<SetScanAreaCommand, DeviceOptionsState>(
                SetScanAreaCommand::Execute, m_DeviceOptionsPanel->GetState());

        if (m_PreviewPanel != nullptr)
        {
            m_DeviceOptionsObserver =
                    new DeviceOptionsObserver(m_DeviceOptionsPanel->GetState(), m_PreviewPanel->GetState());
            m_ObserverManager.AddObserver(m_DeviceOptionsObserver);
        }
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
    if (m_DeviceSelector == nullptr)
    {
        return;
    }

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
    }
    catch (const std::runtime_error &e)
    {
        ZooLib::ShowUserError(m_MainWindow, e.what());
        return;
    }

    device->GetParameters(&m_ScanParameters);
    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        device->CancelScan();
        RestoreScanOptions();
        ZooLib::ShowUserError(m_MainWindow, "Unsupported format");
        return;
    }

    if (m_ScanParameters.depth != 8)
    {
        device->CancelScan();
        RestoreScanOptions();
        ZooLib::ShowUserError(m_MainWindow, "Unsupported depth");
        return;
    }

    if (m_PreviewPanel != nullptr)
    {
        auto height = GetScanHeight();
        auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
        previewPanelUpdater.PrepareForScan(m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, height);
        previewPanelUpdater.SetProgressBounds(0, m_ScanParameters.bytes_per_line * height);

        m_ScanCallbackId = gtk_widget_add_tick_callback(
                GTK_WIDGET(m_MainWindow),
                [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                    auto *localApp = static_cast<App *>(data);
                    localApp->UpdatePreview();
                    return G_SOURCE_CONTINUE;
                },
                this, nullptr);
    }
}

void ZooScan::App::UpdatePreview() const
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
        device->CancelScan();
        RestoreScanOptions();

        previewPanelUpdater.SetProgressCompleted();

        gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), m_ScanCallbackId);
        return;
    }

    previewPanelUpdater.CommitReadBuffer(readLength);
    previewPanelUpdater.IncreaseProgress(readLength);
}

void ZooScan::App::OnScanClicked(GtkWidget *)
{
    const auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    auto fileSaveDialog = gtk_file_chooser_dialog_new(
            "Save Scan", GTK_WINDOW(m_MainWindow), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL,
            "_Save", GTK_RESPONSE_ACCEPT, nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fileSaveDialog), FileWriter::DefaultFileName().c_str());


    auto filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All Files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fileSaveDialog), filter);

    for (auto format: FileWriter::GetFormats())
    {
        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, format->GetName().c_str());

        for (const auto &ext: format->GetExtensions())
        {
            gtk_file_filter_add_pattern(filter, ("*" + ext).c_str());
        }
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fileSaveDialog), filter);
    }

    ConnectGtkSignal(this, &App::OnFileSave, fileSaveDialog, "response");
    gtk_window_present(GTK_WINDOW(fileSaveDialog));
}

void ZooScan::App::OnFileSave(GtkWidget *widget, int responseId)
{
    if (responseId != GTK_RESPONSE_ACCEPT)
    {
        m_ImageFilePath = std::filesystem::path();
        gtk_window_destroy(GTK_WINDOW(widget));
        return;
    }

    g_autoptr(GFile) gFile = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(widget));
    if (gFile == nullptr)
    {
        return;
    }
    auto path = g_file_get_path(gFile);
    if (path == nullptr)
    {
        return;
    }

    m_ImageFilePath = std::filesystem::path(path);
    g_free(path);

    m_FileWriter = FileWriter::GetFormatForPath(m_ImageFilePath);
    if (m_FileWriter == nullptr)
    {
        auto selectedFilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(widget));
        auto filterName = gtk_file_filter_get_name(GTK_FILE_FILTER(selectedFilter));
        m_FileWriter = FileWriter::GetFormatByName(filterName);

        if (m_FileWriter != nullptr)
        {
            m_FileWriter->AddExtension(m_ImageFilePath);
        }
    }

    gtk_window_destroy(GTK_WINDOW(widget));

    if (m_FileWriter == nullptr)
    {
        ZooLib::ShowUserError(m_MainWindow, "Cannot determine image file format");
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
        m_IsScanning = true;
    }
    catch (const std::runtime_error &e)
    {
        ZooLib::ShowUserError(m_MainWindow, e.what());
        return;
    }

    // Must be called after StartScan()
    device->GetParameters(&m_ScanParameters);

    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        device->CancelScan();
        ZooLib::ShowUserError(m_MainWindow, "Unsupported format");
        return;
    }

    if (m_ScanParameters.depth != 8 && m_ScanParameters.depth != 16)
    {
        device->CancelScan();
        ZooLib::ShowUserError(m_MainWindow, "Unsupported depth");
        return;
    }

    if (auto error = m_FileWriter->CreateFile(*this, m_ImageFilePath, GetDeviceOptions(), m_ScanParameters, nullptr);
        error != FileWriter::Error::None)
    {
        device->CancelScan();
        auto errorString = std::string("Failed to create file: ") + m_FileWriter->GetError(error);
        ZooLib::ShowUserError(m_MainWindow, errorString.c_str());
        return;
    }

    auto linesIn1MB = 1024 * 1024 / m_ScanParameters.bytes_per_line;
    m_BufferSize = m_ScanParameters.bytes_per_line * linesIn1MB;
    m_Buffer = static_cast<SANE_Byte *>(calloc(m_BufferSize, sizeof(SANE_Byte)));
    m_WriteOffset = 0;

    auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
    previewPanelUpdater.SetProgressBounds(0, m_BufferSize);

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
    if (m_Buffer == nullptr || m_BufferSize == 0)
    {
        return;
    }

    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    m_FileWriter = FileWriter::GetFormatForPath(m_ImageFilePath);
    if (m_FileWriter == nullptr)
    {
        device->CancelScan();
        ZooLib::ShowUserError(m_MainWindow, "Unsupported file format");
        return;
    }

    auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());

    SANE_Int readLength = 0;
    auto requestedLength = m_BufferSize - m_WriteOffset;
    if (requestedLength > 0)
    {
        if (!device->Read(m_Buffer + m_WriteOffset, requestedLength, &readLength))
        {
            m_IsScanning = false;
            device->CancelScan();
            gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), m_ScanCallbackId);

            previewPanelUpdater.SetProgressCompleted();

            auto dataEnd = m_WriteOffset + readLength;
            auto availableLines = dataEnd / m_ScanParameters.bytes_per_line;
            m_FileWriter->AppendBytes(
                    m_Buffer, availableLines, m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line);
            m_FileWriter->CloseFile();

            free(m_Buffer);
            m_Buffer = nullptr;
            m_BufferSize = 0;
            m_WriteOffset = 0;

            return;
        }
    }

    auto availableBytes = m_WriteOffset + readLength;
    auto availableLines = availableBytes / m_ScanParameters.bytes_per_line;
    auto savedBytes = m_FileWriter->AppendBytes(
            m_Buffer, availableLines, m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line);
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
