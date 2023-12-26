#include "App.h"
#include "ZooFW/SignalSupport.h"
#include "DeviceSelector.h"
#include "Commands/SelectDeviceCommand.h"
#include "DeviceOptionsPanel.h"
#include "AppState.h"
#include "ZooFW/ErrorDialog.h"

#include <sane/sane.h>
#include <stdexcept>
#include <memory>

ZooScan::App::App()
{
    if (SANE_STATUS_GOOD != sane_init(&m_SaneVersion, nullptr))
    {
        sane_exit();
        throw std::runtime_error("Failed to initialize SANE");
    }

    m_AppState = new AppState(&m_State);

    m_Observer = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_Observer);

    m_Dispatcher.RegisterHandler<SelectDeviceCommand, AppState>(SelectDeviceCommand::Execute, m_AppState);
}

ZooScan::App::~App()
{
    delete m_DeviceSelector;
    delete m_DeviceOptionsPanel;

    m_Dispatcher.UnregisterHandler<SelectDeviceCommand>();

    m_ObserverManager.RemoveObserver(m_Observer);
    delete m_Observer;

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
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
    g_object_unref(cssProvider);

    auto *grid = gtk_grid_new();
    gtk_window_set_child(m_MainWindow, grid);

    gtk_grid_set_column_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);

    m_DeviceSelector = new DeviceSelector(&m_Dispatcher, this);
    gtk_grid_attach(GTK_GRID(grid), m_DeviceSelector->RootWidget(), 0, 0, 2, 1);

    m_SettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_grid_attach(GTK_GRID(grid), m_SettingsBox, 0, 1, 1, 1);

    auto button = gtk_button_new_with_label("Preview");
    Zoo::ConnectGtkSignal(this, &App::OnPreviewClicked, button, "clicked");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

    button = gtk_button_new_with_label("Scan");
    Zoo::ConnectGtkSignal(this, &App::OnScanClicked, button, "clicked");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);

    m_PreviewPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, k_PreviewWidth, k_PreviewHeight);
    m_PreviewImage = gtk_image_new_from_pixbuf(m_PreviewPixBuf);
    gtk_widget_set_size_request(m_PreviewImage, k_PreviewWidth, k_PreviewHeight);
    gtk_grid_attach(GTK_GRID(grid), m_PreviewImage, 1, 1, 1, 2);
}

void ZooScan::App::Update(AppState* appState)
{
    static uint64_t lastSeenVersion = 0;

    if (appState != m_AppState)
    {
        throw std::runtime_error("State component mismatch");
    }

    if (m_IsScanning)
    {
        UpdateScanning();
    }
    else if (m_IsPreviewing)
    {
        UpdatePreviewing();
    }

    if (m_AppState->Version() <= lastSeenVersion)
    {
        return;
    }
    lastSeenVersion = m_AppState->Version();

    if (m_DeviceOptionsPanel != nullptr && m_DeviceOptionsPanel->Device() != m_AppState->CurrentDevice())
    {
        gtk_box_remove(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->RootWidget());
        delete m_DeviceOptionsPanel;
        m_DeviceOptionsPanel = nullptr;
    }

    if (m_DeviceOptionsPanel == nullptr && m_SettingsBox != nullptr && m_AppState->CurrentDevice() != nullptr)
    {
        m_DeviceOptionsPanel = new DeviceOptionsPanel(m_AppState->CurrentDevice(), &m_Dispatcher, this);
        gtk_box_append(GTK_BOX(m_SettingsBox), m_DeviceOptionsPanel->RootWidget());
    }
}

void ZooScan::App::RestoreScanOptions()
{
    auto device = m_AppState->CurrentDevice();
    auto options = m_AppState->DeviceOptions();
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
        std::unique_ptr<char[]> valueCString(new char[option->ValueSize() + 1]);
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

int ZooScan::App::GetScanHeight()
{
    auto height = m_ScanParameters.lines;

    if (height == -1)
    {
        auto options = m_AppState->DeviceOptions();
        height = std::ceil(options->GetScanAreaHeight() * options->GetYResolution() / 25.4);
    }

    return height;
}

void ZooScan::App::OnPreviewClicked(GtkWidget *)
{
    auto device = m_AppState->CurrentDevice();
    auto options = m_AppState->DeviceOptions();
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

    auto resolutionDescription = device->GetOptionDescriptor(options->ResolutionIndex());
    SANE_Int resolution;
    if (resolutionDescription->constraint_type == SANE_CONSTRAINT_RANGE)
    {
        resolution = resolutionDescription->constraint.range->min;
    }
    else if (resolutionDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
    {
        resolution = resolutionDescription->constraint.word_list[0];
    }
    else
    {
        resolution = resolutionDescription->type == SANE_TYPE_FIXED ? SANE_FIX(300) : 300;
    }
    device->SetOptionValue(options->ResolutionIndex(), &resolution, nullptr);

    try
    {
        device->StartScan();
        m_IsPreviewing = true;
    }
    catch (const std::runtime_error &e)
    {
        Zoo::ShowUserError(m_MainWindow, e.what());
        return;
    }

    device->GetParameters(&m_ScanParameters);
    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        device->CancelScan();
        RestoreScanOptions();
        Zoo::ShowUserError(m_MainWindow, "Unsupported format");
        return;
    }

    if (m_ScanParameters.depth != 8)
    {
        device->CancelScan();
        RestoreScanOptions();
        Zoo::ShowUserError(m_MainWindow, "Unsupported depth");
        return;
    }

    auto height = GetScanHeight();
    g_print("Image size: %d (%d) x %d\n", m_ScanParameters.pixels_per_line, m_ScanParameters.bytes_per_line, height);

    m_FullImageSize = m_ScanParameters.bytes_per_line * height;
    m_FullImage = static_cast<SANE_Byte *>(calloc(m_FullImageSize, 1));

    m_Offset = 0;
}

void ZooScan::App::DrawRGBPreview()
{
    auto width = m_ScanParameters.pixels_per_line;
    auto height = GetScanHeight();
    auto scaleW = double(k_PreviewWidth)/width;
    auto scaleH = double(k_PreviewHeight)/height;
    auto scale = std::min(scaleW, scaleH);
    auto previewWidth = k_PreviewWidth * scale / scaleW;
    auto previewHeight = k_PreviewHeight * scale / scaleH;

    auto *pixbuf = gdk_pixbuf_new_from_data(m_FullImage, GDK_COLORSPACE_RGB, false, 8,
                                            width, height,m_ScanParameters.bytes_per_line,nullptr, nullptr);
    gdk_pixbuf_fill(m_PreviewPixBuf, 0);
    gdk_pixbuf_scale(pixbuf, m_PreviewPixBuf, 0, 0, int(previewWidth), int(previewHeight), 0., 0., scale, scale,
                     GDK_INTERP_NEAREST);
    g_object_unref(pixbuf);
    gtk_image_set_from_pixbuf(GTK_IMAGE(m_PreviewImage), m_PreviewPixBuf);
    gtk_widget_queue_draw(m_PreviewImage);
}

void ZooScan::App::UpdatePreviewing()
{
    auto device = m_AppState->CurrentDevice();
    SANE_Int readLength = 0;
    auto requestedLength = int(std::min(1024UL*1024UL, m_FullImageSize - m_Offset));
    if (requestedLength == 0 || !device->Read(m_FullImage + m_Offset, requestedLength, &readLength))
    {
        g_print("Done reading data\n");
        m_IsPreviewing = false;
        device->CancelScan();
        RestoreScanOptions();

        DrawRGBPreview();

        free (m_FullImage);
        m_FullImage = nullptr;
        m_FullImageSize = 0;

        return;
    }

    g_print("Read data %d\n", readLength);
    m_Offset += readLength;
    DrawRGBPreview();
}

void ZooScan::App::OnScanClicked(GtkWidget *)
{
    auto device = m_AppState->CurrentDevice();

    try
    {
        device->StartScan();
        m_IsScanning = true;
    }
    catch (const std::runtime_error &e)
    {
        Zoo::ShowUserError(m_MainWindow, e.what());
        return;
    }

    device->GetParameters(&m_ScanParameters);
    if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
    {
        device->CancelScan();
        Zoo::ShowUserError(m_MainWindow, "Unsupported format");
        return;
    }

    if (m_ScanParameters.depth != 8 && m_ScanParameters.depth != 16)
    {
        device->CancelScan();
        Zoo::ShowUserError(m_MainWindow, "Unsupported depth");
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
    auto device = m_AppState->CurrentDevice();

    SANE_Int readLength = 0;
    auto requestedLength = int(std::min(1024UL*1024UL, m_FullImageSize - m_Offset));
    if (requestedLength == 0 || !device->Read(m_FullImage + m_Offset, requestedLength, &readLength))
    {
        g_print("Done reading data\n");
        m_IsScanning = false;
        device->CancelScan();

        // TODO Save image

        free (m_FullImage);
        m_FullImage = nullptr;
        m_FullImageSize = 0;

        return;
    }

    g_print("Read data %d\n", readLength);
    m_Offset += readLength;
}
