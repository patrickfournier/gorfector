#pragma once

#include <gtk/gtk.h>
#include <sane/sane.h>
#include <tuple>

#include "AppState.hpp"
#include "DeviceOptionsState.hpp"
#include "DeviceSelector.hpp"
#include "OutputOptionsState.hpp"
#include "ZooLib/Application.hpp"

namespace ZooScan
{
    class PresetPanel;
    class FileWriter;
    class DeviceOptionsObserver;
    class ScanOptionsPanel;
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

        ScanOptionsPanel *m_ScanOptionsPanel{};
        PreviewPanel *m_PreviewPanel{};
        PresetPanel *m_PresetPanel{};

        GtkWidget *m_SettingsBox{};

        GtkWidget *m_PreviewButton{};
        GtkWidget *m_ScanButton{};
        GtkWidget *m_CancelButton{};

        SANE_Parameters m_ScanParameters{};
        int32_t m_BufferSize{};
        SANE_Byte *m_Buffer{};
        int32_t m_WriteOffset{};
        guint m_ScanCallbackId{};

        FileWriter *m_FileWriter{};
        std::filesystem::path m_ImageFilePath{};

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
            return m_DeviceSelectorState->GetDeviceByName(m_AppState->GetCurrentDeviceName());
        }

        [[nodiscard]] int GetScanHeight() const;

        void OnCancelClicked(GtkWidget *);

        void OnPreviewClicked(GtkWidget *widget);
        void UpdatePreview();
        void RestoreScanOptions();
        void StopPreview();

        void CheckFileOutputOptionsAndScan(const OutputOptionsState *scanOptions);
        void OnOverwriteAlertResponse(AdwAlertDialog *alert, gchar *response);
        bool SelectFileWriter();
        void OnScanClicked(GtkWidget *widget);
        void StartScan();
        void UpdateScan();
        void StopScan(bool completed);

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

        [[nodiscard]] const DeviceSelectorState *GetDeviceSelectorState() const
        {
            return m_DeviceSelectorState;
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

        [[nodiscard]] DeviceOptionsState *GetDeviceOptions() const;
        [[nodiscard]] OutputOptionsState *GetOutputOptions();
    };
}
