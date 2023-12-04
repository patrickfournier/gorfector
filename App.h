#pragma once

#include <gtk/gtk.h>
#include <sane/sane.h>
#include <tuple>
#include "ZooFW/Application.h"
#include "DeviceSelector.h"
#include "DeviceSettingsPanel.h"
#include "AppState.h"

namespace ZooScan
{
    class App : public Zoo::Application
    {
        Zoo::CommandDispatcher m_Dispatcher{};

        Zoo::State m_State{};
        AppState *m_AppState{};

        ViewUpdateObserver<App, AppState> *m_Observer;

        SANE_Int m_SaneVersion{};

        DeviceSelector *m_DeviceSelector{};

        GtkWidget *m_SettingsBox;
        GdkPixbuf* m_PreviewPixBuf{};
        GtkWidget* m_PreviewImage{};

        DeviceSettingsPanel *m_DeviceSettingsPanel{};

    protected:
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

        void OnScanClicked(GtkWidget *widget);

    public:
        App();

        ~App() override;

        DeviceSelector *CreateDeviceSelector();

        void Update();
    };

}
