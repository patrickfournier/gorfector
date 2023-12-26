#pragma once

#include <gtk/gtk.h>
#include <sane/sane.h>
#include <tuple>
#include "ZooFW/Application.h"
#include "DeviceSelector.h"
#include "AppState.h"

namespace ZooScan
{
    class DeviceOptionsPanel;

    class App : public Zoo::Application
    {
    private:
        const int k_PreviewWidth = 750;
        const int k_PreviewHeight = 1000;

        Zoo::CommandDispatcher m_Dispatcher{};

        Zoo::State m_State{};
        AppState *m_AppState{};

        ViewUpdateObserver<App, AppState> *m_Observer;

        SANE_Int m_SaneVersion{};

        DeviceSelector *m_DeviceSelector{};

        GtkWidget *m_SettingsBox{};
        GdkPixbuf* m_PreviewPixBuf{};
        GtkWidget* m_PreviewImage{};

        DeviceOptionsPanel *m_DeviceOptionsPanel{};

        std::string GetApplicationId() override
        {
            return "com.patrickfournier.zooscan";
        }

        GApplicationFlags GetApplicationFlags() override
        {
            return G_APPLICATION_FLAGS_NONE;
        }

        std::string GetMainWindowTitle() override
        {
            return "ZooScan";
        }

        std::tuple<int, int> GetMainWindowSize() override
        {
            return std::make_tuple(640, 480);
        }

        void PopulateMainWindow() override;

        SANE_Parameters m_ScanParameters{};
        uint64_t m_FullImageSize{};
        SANE_Byte * m_FullImage{};
        uint64_t m_Offset{};

        int GetScanHeight();

        bool m_IsPreviewing{};
        void OnPreviewClicked(GtkWidget *widget);
        void UpdatePreviewing();
        void DrawRGBPreview();
        void RestoreScanOptions();

        bool m_IsScanning{};
        void OnScanClicked(GtkWidget *widget);
        void UpdateScanning();

    public:
        App();

        ~App() override;

        AppState* GetAppState()
        { return m_AppState; }

        void Update(AppState* appState);
    };
}
