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
    class FileWriter;
    class DeviceOptionsObserver;
    class DeviceOptionsPanel;
    class DeviceSelectorObserver;
    class PreviewPanel;

    class App : public ZooLib::Application
    {
    public:
        static constexpr auto k_ApplicationId = "com.patrickfournier.zooscan";

    private:
        AppState *m_AppState{};
        DeviceSelectorState *m_DeviceSelectorState{};

        ViewUpdateObserver<App, AppState> *m_ViewUpdateObserver{};
        DeviceSelectorObserver *m_DeviceSelectorObserver{};
        DeviceOptionsObserver *m_DeviceOptionsObserver{};

        DeviceOptionsPanel *m_DeviceOptionsPanel{};
        PreviewPanel *m_PreviewPanel{};

        GtkWidget *m_SettingsBox{};

        GtkWidget *m_PreviewButton{};
        GtkWidget *m_ScanButton{};
        GtkWidget *m_CancelButton{};

        SANE_Parameters m_ScanParameters{};
        int32_t m_BufferSize{};
        SANE_Byte *m_Buffer{};
        int32_t m_WriteOffset{};
        guint m_ScanCallbackId{};

        std::filesystem::path m_ImageFilePath{};
        FileWriter *m_FileWriter{};

        [[nodiscard]] GApplicationFlags GetApplicationFlags() override
        {
            return G_APPLICATION_DEFAULT_FLAGS;
        }

        [[nodiscard]] std::string GetMainWindowTitle() override
        {
            return "ZooScan";
        }

        [[nodiscard]] std::tuple<int, int> GetMainWindowSize() override
        {
            return std::make_tuple(640, 480);
        }

        void OnActivate(GtkApplication *app) override;
        GtkWidget *CreateContent() override;
        void PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder) override;

        [[nodiscard]] SaneDevice *GetDevice() const
        {
            if (m_DeviceSelectorState == nullptr)
            {
                return nullptr;
            }
            return m_DeviceSelectorState->GetDeviceByName(m_AppState->GetOptionPanelDeviceName());
        }

        [[nodiscard]] int GetScanHeight() const;

        void OnPreviewClicked(GtkWidget *widget);
        void UpdatePreview();
        void OnCancelClicked(GtkWidget *);
        void RestoreScanOptions() const;

        void OnScanClicked(GtkWidget *widget);
        void StartScan();
        void UpdateScan();
        void StopScan();

        [[nodiscard]] const std::string &GetSelectorDeviceName() const;

        [[nodiscard]] int GetSelectorSaneInitId() const;

        void AboutDialog(GSimpleAction *action = nullptr, GVariant *parameter = nullptr);
        void SelectDeviceDialog(GSimpleAction *action, GVariant *parameter);
        void PreferenceDialog(GSimpleAction *action, GVariant *parameter);

    public:
        App(int argc, char **argv);

        ~App() override;

        [[nodiscard]] std::string GetApplicationId() const override
        {
            return k_ApplicationId;
        }

        [[nodiscard]] const AppState *GetAppState() const
        {
            return m_AppState;
        }

        [[nodiscard]] AppState *GetAppState()
        {
            return m_AppState;
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions);

        [[nodiscard]] SaneDevice *GetDeviceByName(const std::string &deviceName) const
        {
            if (m_DeviceSelectorState == nullptr)
            {
                return nullptr;
            }

            return m_DeviceSelectorState->GetDeviceByName(deviceName);
        }

        [[nodiscard]] const DeviceOptionsState *GetDeviceOptions() const;

        void OnFileSave(GFile *file);
    };
}
