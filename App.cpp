#include <memory>
#include <sane/sane.h>
#include <stdexcept>

#include <tiffio.h>

#include "App.hpp"
#include "AppState.hpp"
#include "Commands/SetScanAreaCommand.hpp"
#include "DeviceOptionsObserver.hpp"
#include "DeviceOptionsPanel.hpp"
#include "DeviceSelector.hpp"
#include "DeviceSelectorObserver.hpp"
#include "PreviewPanel.hpp"
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

    delete[] m_FullImage;

    sane_exit();
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

    gtk_grid_set_column_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);

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
        UpdateScanning();
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
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : value;
        device->SetOptionValue(options->TLXIndex(), &fValue, nullptr);
    }
    if (options->TLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->TLYIndex());
        double value = maxScanArea.y;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : value;
        device->SetOptionValue(options->TLYIndex(), &fValue, nullptr);
    }
    if (options->BRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->BRXIndex());
        double value = maxScanArea.x + maxScanArea.width;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : value;
        device->SetOptionValue(options->BRXIndex(), &fValue, nullptr);
    }
    if (options->BRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = device->GetOptionDescriptor(options->BRYIndex());
        double value = maxScanArea.y + maxScanArea.height;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : value;
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

    auto height = GetScanHeight();
    g_print("Image size: %d (%d) x %d\n", m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, height);

    if (m_PreviewPanel != nullptr)
    {
        auto previewPanelUpdater = PreviewState::Updater(m_PreviewPanel->GetState());
        previewPanelUpdater.PrepareForScan(m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, height);

        m_UpdatePreviewCallbackId = gtk_widget_add_tick_callback(
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

        previewPanelUpdater.ResetReadBuffer();

        gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), m_UpdatePreviewCallbackId);
        return;
    }

    previewPanelUpdater.CommitReadBuffer(readLength);
}

void ZooScan::App::OnScanClicked(GtkWidget *)
{
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

    auto height = GetScanHeight();
    g_print("Image size: %d (%d) x %d\n", m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, height);

    m_FullImageSize = m_ScanParameters.bytes_per_line * height;
    m_FullImage = static_cast<SANE_Byte *>(calloc(m_FullImageSize, 1));

    m_Offset = 0;
}

void ZooScan::App::UpdateScanning()
{
    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    SANE_Int readLength = 0;
    auto requestedLength = static_cast<int>(std::min(1024UL * 1024UL, m_FullImageSize - m_Offset));
    if (requestedLength == 0 || !device->Read(m_FullImage + m_Offset, requestedLength, &readLength))
    {
        g_print("Done reading data\n");
        m_IsScanning = false;
        device->CancelScan();

        auto options = GetDeviceOptions();
        int xResolution = options == nullptr               ? 0
                          : options->GetXResolution() == 0 ? options->GetResolution()
                                                           : options->GetXResolution();
        int yResolution = options == nullptr               ? 0
                          : options->GetYResolution() == 0 ? options->GetResolution()
                                                           : options->GetYResolution();

        TIFF *tifFile = TIFFOpen("test.tiff", "w");

        TIFFSetField(tifFile, TIFFTAG_IMAGEWIDTH, m_ScanParameters.pixels_per_line);
        TIFFSetField(tifFile, TIFFTAG_IMAGELENGTH, m_ScanParameters.lines);
        TIFFSetField(tifFile, TIFFTAG_SAMPLESPERPIXEL, m_ScanParameters.format == SANE_FRAME_RGB ? 3 : 1);
        TIFFSetField(tifFile, TIFFTAG_BITSPERSAMPLE, m_ScanParameters.depth);
        TIFFSetField(tifFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tifFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(
                tifFile, TIFFTAG_PHOTOMETRIC,
                m_ScanParameters.format == SANE_FRAME_RGB ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tifFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
        TIFFSetField(tifFile, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tifFile, TIFFTAG_XRESOLUTION, xResolution);
        TIFFSetField(tifFile, TIFFTAG_YRESOLUTION, yResolution);
        TIFFSetField(tifFile, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

        for (auto i = 0; i < m_ScanParameters.lines; i++)
        {
            TIFFWriteScanline(tifFile, m_FullImage + i * m_ScanParameters.bytes_per_line, i, 0);
        }

        TIFFClose(tifFile);

        free(m_FullImage);
        m_FullImage = nullptr;
        m_FullImageSize = 0;

        return;
    }

    g_print("Read data %d\n", readLength);
    m_Offset += readLength;
}
