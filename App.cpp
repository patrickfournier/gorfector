#include "App.h"
#include "ZooFW/SignalSupport.h"
#include "DeviceSelector.h"
#include "Commands/OpenDeviceCommand.h"
#include "DeviceSettingsPanel.h"
#include "AppState.h"

#include <sane/sane.h>
#include <stdexcept>

ZooScan::App::App()
{
    if (SANE_STATUS_GOOD != sane_init(&m_SaneVersion, nullptr))
    {
        sane_exit();
        throw std::runtime_error("Failed to initialize SANE");
    }

    m_AppState = new AppState();
    m_State.AddStateComponent(m_AppState);
    m_Observer = new ViewUpdateObserver(this, m_AppState);
    m_ObserverManager.AddObserver(m_Observer);

    m_Dispatcher.RegisterHandler<OpenDeviceCommand, AppState>(OpenDeviceCommand::Execute, m_AppState);
}

ZooScan::App::~App()
{
    delete m_DeviceSelector;

    m_Dispatcher.UnregisterHandler<OpenDeviceCommand>();

    m_ObserverManager.RemoveObserver(m_Observer);
    delete m_Observer;

    m_State.RemoveStateComponent(m_AppState);
    delete m_AppState;

    sane_exit();
}

ZooScan::DeviceSelector* ZooScan::App::CreateDeviceSelector()
{
    return new DeviceSelector(&m_Dispatcher, this);
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

    m_DeviceSelector = CreateDeviceSelector();
    m_DeviceSelector->Update();
    gtk_grid_attach(GTK_GRID(grid), m_DeviceSelector->RootWidget(), 0, 0, 2, 1);

    m_SettingsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_grid_attach(GTK_GRID(grid), m_SettingsBox, 0, 1, 1, 1);

    auto button = gtk_button_new_with_label("Scan");
    Zoo::ConnectGtkSignal(this, &App::OnScanClicked, button, "clicked");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

    m_PreviewPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, 450, 700);
    m_PreviewImage = gtk_image_new_from_pixbuf(m_PreviewPixBuf);
    gtk_widget_set_size_request(m_PreviewImage, 450, 700);
    gtk_grid_attach(GTK_GRID(grid), m_PreviewImage, 1, 1, 1, 2);
}

void ZooScan::App::Update()
{
    static u_int64_t lastSeenVersion = 0;

    if (m_AppState->Version() <= lastSeenVersion)
    {
        return;
    }
    lastSeenVersion = m_AppState->Version();

    if (m_DeviceSettingsPanel != nullptr && m_DeviceSettingsPanel->Device() != nullptr)
    {
        gtk_box_remove(GTK_BOX(m_SettingsBox), m_DeviceSettingsPanel->RootWidget());
        delete m_DeviceSettingsPanel;
        m_DeviceSettingsPanel = nullptr;
    }

    if (m_DeviceSettingsPanel == nullptr && m_SettingsBox != nullptr && m_AppState->CurrentDevice() != nullptr)
    {
        m_DeviceSettingsPanel = new DeviceSettingsPanel(m_AppState->CurrentDevice(), GTK_BOX(m_SettingsBox), &m_Dispatcher, this);
        gtk_box_append(GTK_BOX(m_SettingsBox), m_DeviceSettingsPanel->RootWidget());
    }
}

void ZooScan::App::OnScanClicked(GtkWidget *widget)
{
    int width = 0;
    int height = 0;
    SANE_Handle handle;

    if (width > 0 && height > 0)
    {
        float xScale = 450.0f / width;
        float yScale = 700.0f / height;
        float scale = xScale < yScale ? xScale : yScale;

        if (sane_start(handle) != SANE_STATUS_GOOD)
        {
            throw std::runtime_error("Failed to start scan");
        }

        auto *fullImage = static_cast<SANE_Byte *>(malloc(width*height));
        for (int i = 0; i < width*height; i++)
        {
            fullImage[i] = 0;
        }

        int bufferLength = 65536;
        auto buffer = static_cast<SANE_Byte *>(malloc(bufferLength));
        uint64_t offset = 0;
        SANE_Int readLength = 0;
        while (sane_read(handle, buffer, bufferLength, &readLength) == SANE_STATUS_GOOD)
        {
            g_print("Read data %d\n", readLength);
            readLength = 0;

            memcpy(fullImage + offset, buffer, readLength);
            offset += readLength;

            auto *pixbuf = gdk_pixbuf_new_from_data(fullImage, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, nullptr, nullptr);
            gdk_pixbuf_scale(pixbuf, m_PreviewPixBuf, 0, 0, width * scale, height * scale, 0, 0, scale, scale, GDK_INTERP_BILINEAR);
            g_object_unref(pixbuf);
            gtk_widget_queue_draw(m_PreviewImage);
        }
        sane_cancel(handle);

        free(fullImage);
    }
}
