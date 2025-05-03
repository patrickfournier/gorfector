#pragma once

#include <gtk/gtk.h>
#include <sane/sane.h>
#include <tuple>

#include "AppState.hpp"
#include "DeviceOptionsState.hpp"
#include "DeviceSelector.hpp"
#include "OutputOptionsState.hpp"
#include "ZooLib/Application.hpp"

namespace Gorfector
{
    class ScanProcess;
    class ScanListPanel;
    class PresetPanel;
    class FileWriter;
    class DeviceOptionsObserver;
    class ScanOptionsPanel;
    class DeviceSelectorObserver;
    class PreviewPanel;

    class App : public ZooLib::Application
    {
    public:
        static constexpr auto k_ApplicationId = "com.patrickfournier.gorfector";
        static constexpr auto k_ApplicationName = "Gorfector";

    private:
        AppState *m_AppState{};
        DeviceSelectorState *m_DeviceSelectorState{};

        ViewUpdateObserver<App, AppState> *m_ViewUpdateObserver{};
        DeviceSelectorObserver *m_DeviceSelectorObserver{};
        DeviceOptionsObserver *m_DeviceOptionsObserver{};

        ScanOptionsPanel *m_ScanOptionsPanel{};
        PreviewPanel *m_PreviewPanel{};
        PresetPanel *m_PresetPanel{};
        ScanListPanel *m_ScanListPanel{};

        GtkWidget *m_LeftPaned{};
        GtkWidget *m_RightPaned{};

        GtkWidget *m_SettingsBox{};

        GtkWidget *m_PreviewButton{};
        GtkWidget *m_ScanButton{};
        GtkWidget *m_CancelButton{};
        GtkWidget *m_AddToScanListButton{};

        ScanProcess *m_ScanProcess{};

        [[nodiscard]] GApplicationFlags GetApplicationFlags() override
        {
            return G_APPLICATION_DEFAULT_FLAGS;
        }

        [[nodiscard]] std::string GetMainWindowTitle() override
        {
            return k_ApplicationName;
        }

        [[nodiscard]] std::tuple<int, int> GetMainWindowSize() override
        {
            return std::make_tuple(640, 480);
        }

        void OnActivate(GtkApplication *app) override;
        void OnPanelResized(GtkWidget *widget);
        GtkWidget *BuildUI() override;
        void BuildScanListUI();
        void RemoveScanListUI();
        void PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder) override;

        [[nodiscard]] SaneDevice *GetDevice() const
        {
            if (m_DeviceSelectorState == nullptr)
            {
                return nullptr;
            }
            return m_DeviceSelectorState->GetDeviceByName(m_AppState->GetCurrentDeviceName());
        }

        void OnCancelClicked(GtkWidget *);

        void OnPreviewClicked(GtkWidget *widget);

        bool CheckFileOutputOptions(const OutputOptionsState *scanOptions) const;
        void ScanToFile(const OutputOptionsState *scanOptions);
        void OnOverwriteAlertResponse(AdwAlertDialog *alert, gchar *response);
        FileWriter *SelectFileWriter(const std::string &path) const;
        void OnAddToScanListClicked(GtkWidget *);
        void OnScanClicked(GtkWidget *widget);
        void StartScan(FileWriter *fileWriter, const std::filesystem::path &imageFilePath);

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

        [[nodiscard]] std::string GetApplicationName() const override
        {
            return k_ApplicationName;
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
