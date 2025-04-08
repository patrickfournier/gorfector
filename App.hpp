#pragma once

#include <gtk/gtk.h>
#include <sane/sane.h>
#include <tuple>

#include "AppState.hpp"
#include "DeviceOptionsState.hpp"
#include "DeviceSelector.hpp"
#include "ZooLib/Application.hpp"

namespace ZooScan
{
    class DeviceOptionsObserver;
    class DeviceOptionsPanel;
    class DeviceSelectorObserver;
    class PreviewPanel;

    class App : public ZooLib::Application
    {
    private:
        ZooLib::CommandDispatcher m_Dispatcher{};

        ZooLib::State m_State{};
        AppState *m_AppState{};

        ViewUpdateObserver<App, AppState> *m_ViewUpdateObserver{};
        DeviceSelectorObserver *m_DeviceSelectorObserver{};
        DeviceOptionsObserver *m_DeviceOptionsObserver{};

        DeviceSelector *m_DeviceSelector{};
        DeviceOptionsPanel *m_DeviceOptionsPanel{};
        PreviewPanel *m_PreviewPanel{};

        GtkWidget *m_SettingsBox{};

        SANE_Parameters m_ScanParameters{};
        uint64_t m_ScannedImageSize{};
        SANE_Byte *m_ScannedImage{};
        uint64_t m_Offset{};
        bool m_IsScanning{};
        guint m_ScanCallbackId;

        std::filesystem::path m_ImageFilePath{};

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
            return std::make_tuple(-1, -1);
        }

        void PopulateMainWindow() override;

        SaneDevice *GetDevice() const
        {
            if (m_DeviceSelector == nullptr)
            {
                return nullptr;
            }
            return m_DeviceSelector->GetState()->GetDeviceByName(m_AppState->GetOptionPanelDeviceName());
        }

        int GetScanHeight() const;

        void OnPreviewClicked(GtkWidget *widget);
        void UpdatePreview() const;
        void RestoreScanOptions() const;

        void OnScanClicked(GtkWidget *widget);
        void OnFileSave(GtkWidget *widget, int responseId);
        void StartScan();
        void UpdateScan();

        const std::string &GetSelectorDeviceName() const;

        int GetSelectorSaneInitId() const;

    public:
        App();

        ~App() override;

        const AppState *GetAppState() const
        {
            return m_AppState;
        }

        AppState *GetAppState()
        {
            return m_AppState;
        }

        void Update(u_int64_t lastSeenVersion);

        SaneDevice *GetDeviceByName(const std::string &deviceName) const
        {
            if (m_DeviceSelector == nullptr)
            {
                return nullptr;
            }

            return m_DeviceSelector->GetState()->GetDeviceByName(deviceName);
        }

        const DeviceOptionsState *GetDeviceOptions() const;
    };
}
