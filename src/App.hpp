#pragma once

#include <gtk/gtk.h>
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

    /**
     * \class App
     * \brief Main application class for the Gorfector application.
     *
     * This class manages the application's lifecycle, UI components, and interactions
     * with the scanning devices and user interface.
     */
    class App : public ZooLib::Application
    {
    public:
        /**
         * \brief The application name displayed to the user.
         */
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

        ScanProcess *m_ScanProcess{};

        /**
         * \brief Constructor for the App class.
         * \param argc The number of command-line arguments.
         * \param argv The array of command-line arguments.
         * \param testMode If true, enables test mode for automated testing with
         *                 test actions that can be pushed to the application.
         */
        App(int argc, char **argv, bool testMode = false);

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

        void OnScanClicked(GtkWidget *widget);
        void OnCancelClicked(GtkWidget *);
        void OnPreviewClicked(GtkWidget *widget);

        void ScanToFile(const OutputOptionsState *scanOptions);
        void OnOverwriteAlertResponse(AdwAlertDialog *alert, gchar *response);
        FileWriter *SelectFileWriter(const std::string &path) const;
        void StartScan(FileWriter *fileWriter, const std::filesystem::path &imageFilePath);

        [[nodiscard]] const std::string &GetSelectorDeviceName() const;

        [[nodiscard]] int GetSelectorSaneInitId() const;

        void ShowAboutDialog(GSimpleAction *action = nullptr, GVariant *parameter = nullptr);
        void ShowSelectDeviceDialog(GSimpleAction *action, GVariant *parameter);
        void ShowPreferenceDialog(GSimpleAction *action, GVariant *parameter);
        void ShowHelp(GSimpleAction *action, GVariant *parameter);

    public:
        static App *Create(int argc, char **argv, bool testMode = false)
        {
            auto app = new App(argc, argv, testMode);
            app->Initialize();
            return app;
        }

        /**
         * \brief Destructor for the App class.
         */
        ~App() override;

        /**
         * \brief Retrieves the application name.
         * \return The application name as a string.
         */
        [[nodiscard]] std::string GetApplicationName() const override
        {
            return k_ApplicationName;
        }

        /**
         * \brief Retrieves the application state.
         * \return A pointer to the AppState object.
         */
        [[nodiscard]] AppState *GetAppState()
        {
            return m_AppState;
        }

        /**
         * \brief Retrieves the device selector state.
         * \return A pointer to the DeviceSelectorState object.
         */
        [[nodiscard]] const DeviceSelectorState *GetDeviceSelectorState() const
        {
            return m_DeviceSelectorState;
        }

        /**
         * \brief Updates the application UI from the state components.
         * \param lastSeenVersions A vector of last seen version identifiers for the state components.
         */
        void Update(const std::vector<uint64_t> &lastSeenVersions);

        /**
         * \brief Retrieves a SaneDevice by its name.
         * \param deviceName The name of the device.
         * \return A pointer to the SaneDevice object.
         */
        [[nodiscard]] SaneDevice *GetDeviceByName(const std::string &deviceName) const
        {
            if (m_DeviceSelectorState == nullptr)
            {
                return nullptr;
            }

            return m_DeviceSelectorState->GetDeviceByName(deviceName);
        }

        /**
         * \brief Retrieves the device options state.
         * \return A pointer to the DeviceOptionsState object.
         */
        [[nodiscard]] DeviceOptionsState *GetDeviceOptions() const;

        /**
         * \brief Retrieves the output options state.
         * \return A pointer to the OutputOptionsState object.
         */
        [[nodiscard]] OutputOptionsState *GetOutputOptions();

        /**
         * \brief Retrieves the preview panel.
         * \return A pointer to the PreviewPanel object.
         */
        [[nodiscard]] PreviewPanel *GetPreviewPanel() const
        {
            return m_PreviewPanel;
        }

        /**
         * \brief Validates the file output options provided by the user.
         *
         * This method checks if the output directory and file name are valid and ensures
         * that a suitable file writer can be selected for the specified file name.
         * If any validation fails, it displays an appropriate error message to the user
         * and navigates to the relevant UI page for correction.
         *
         * \param scanOptions A pointer to the `OutputOptionsState` object containing the file output options.
         * \return `true` if the file output options are valid, `false` otherwise.
         */
        bool CheckFileOutputOptions(const OutputOptionsState *scanOptions) const;
    };
}
