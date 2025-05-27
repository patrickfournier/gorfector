#pragma once

#include "App.hpp"
#include "Commands/CreatePresetCommand.hpp"
#include "Commands/DeletePresetCommand.hpp"
#include "Commands/SetPresetExpanded.hpp"
#include "Commands/UpdatePresetCommand.hpp"
#include "CurrentDeviceObserver.hpp"
#include "PresetPanelDialogs.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/GtkUtils.hpp"
#include "ZooLib/View.hpp"

namespace Gorfector
{
    class PresetPanel : public ZooLib::View
    {
        App *m_App;
        ZooLib::CommandDispatcher m_Dispatcher{};

        PresetPanelState *m_PresetPanelState{};
        ViewUpdateObserver<PresetPanel, PresetPanelState> *m_ViewUpdateObserver{};
        CurrentDeviceObserver<PresetPanelState> *m_CurrentDeviceObserver;

        std::string m_CurrentDeviceModel{};
        std::string m_CurrentDeviceVendor{};

        GtkWidget *m_RootWidget{};
        GtkWidget *m_Expander{};
        GtkWidget *m_CreatePresetButton{};
        GtkWidget *m_ListBox{};
        std::vector<std::string> m_DisplayedPresetNames{};

        PresetPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app)
            : m_App(app)
            , m_Dispatcher(parentDispatcher)
        {
            m_PresetPanelState = new PresetPanelState(m_App->GetState());
            m_App->GetState()->LoadFromPreferencesFile(m_PresetPanelState);

            BuildUI();

            m_ViewUpdateObserver = new ViewUpdateObserver(this, m_PresetPanelState);
            m_App->GetObserverManager()->AddObserver(m_ViewUpdateObserver);

            m_CurrentDeviceObserver = new CurrentDeviceObserver(
                    m_App->GetAppState(), m_App->GetDeviceSelectorState(), m_PresetPanelState);
            m_App->GetObserverManager()->AddObserver(m_CurrentDeviceObserver);

            m_Dispatcher.RegisterHandler(SetPresetExpanded::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(CreatePresetCommand::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(DeletePresetCommand::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(UpdatePresetCommand::Execute, m_PresetPanelState);
        }

        void OnExpanderExpanded(GtkWidget *widget)
        {
            m_Dispatcher.Dispatch(SetPresetExpanded(gtk_expander_get_expanded(GTK_EXPANDER(widget))));
        }

        void
        GatherPresetValues(nlohmann::json &preset, bool saveScannerSettings, bool saveScanArea, bool saveOutputSettings)
        {
            auto scannerSettings = nlohmann::json{};
            to_json(scannerSettings, *m_App->GetDeviceOptions());

            if (saveScanArea)
            {
                preset[PresetPanelState::k_ScanAreaKey] = {
                        {DeviceOptionsState::k_TlxKey,
                         scannerSettings[DeviceOptionsState::k_OptionsKey][DeviceOptionsState::k_TlxKey]},
                        {DeviceOptionsState::k_TlyKey,
                         scannerSettings[DeviceOptionsState::k_OptionsKey][DeviceOptionsState::k_TlyKey]},
                        {DeviceOptionsState::k_BrxKey,
                         scannerSettings[DeviceOptionsState::k_OptionsKey][DeviceOptionsState::k_BrxKey]},
                        {DeviceOptionsState::k_BryKey,
                         scannerSettings[DeviceOptionsState::k_OptionsKey][DeviceOptionsState::k_BryKey]},
                };
            }
            if (saveScannerSettings)
            {
                preset[PresetPanelState::k_ScannerSettingsKey] = scannerSettings;

                preset[PresetPanelState::k_ScannerSettingsKey][DeviceOptionsState::k_OptionsKey].erase(
                        DeviceOptionsState::k_TlxKey);
                preset[PresetPanelState::k_ScannerSettingsKey][DeviceOptionsState::k_OptionsKey].erase(
                        DeviceOptionsState::k_TlyKey);
                preset[PresetPanelState::k_ScannerSettingsKey][DeviceOptionsState::k_OptionsKey].erase(
                        DeviceOptionsState::k_BrxKey);
                preset[PresetPanelState::k_ScannerSettingsKey][DeviceOptionsState::k_OptionsKey].erase(
                        DeviceOptionsState::k_BryKey);
            }
            if (saveOutputSettings)
            {
                preset[PresetPanelState::k_OutputSettingsKey] = {};
                to_json(preset[PresetPanelState::k_OutputSettingsKey], *m_App->GetOutputOptions());
            }
        }

    public:
        /**
         * \brief Creates a new instance of a `PresetPanel` class.
         *
         * This static method allocates and initializes a new `PresetPanel` instance, ensuring that
         * the `PostCreateView` method is called to set up the destroy signal.
         *
         * \param parentDispatcher Pointer to the parent command dispatcher.
         * \param app Pointer to the application instance.
         * \return A pointer to the newly created `PresetPanel` instance.
         */
        static PresetPanel *Create(ZooLib::CommandDispatcher *parentDispatcher, App *app)
        {
            auto view = new PresetPanel(parentDispatcher, app);
            view->PostCreateView();
            return view;
        }

        ~PresetPanel() override
        {
            m_Dispatcher.UnregisterHandler<SetPresetExpanded>();
            m_Dispatcher.UnregisterHandler<CreatePresetCommand>();
            m_Dispatcher.UnregisterHandler<DeletePresetCommand>();
            m_Dispatcher.UnregisterHandler<UpdatePresetCommand>();

            m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);
            m_App->GetObserverManager()->RemoveObserver(m_CurrentDeviceObserver);

            delete m_PresetPanelState;
        }

        [[nodiscard]] PresetPanelState *GetState() const
        {
            return m_PresetPanelState;
        }

        void BuildUI();

        GtkWidget *CreatePresetListItem(const char *itemName);

        [[nodiscard]] AdwApplicationWindow *GetMainWindow()
        {
            return m_App->GetMainWindow();
        }

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        void CreatePreset(
                const std::string &presetName, bool saveScannerSettings, bool saveScanArea, bool saveOutputSettings)
        {
            auto preset = nlohmann::json{};
            preset[PresetPanelState::k_PresetNameKey] = presetName;
            GatherPresetValues(preset, saveScannerSettings, saveScanArea, saveOutputSettings);
            m_Dispatcher.Dispatch(CreatePresetCommand(preset));
        }

        [[nodiscard]] const nlohmann::json *GetPreset(int presetRowIndex) const
        {
            auto presetId = m_DisplayedPresetNames[presetRowIndex];
            return m_PresetPanelState->GetPreset(presetId);
        }

        void OnApplyPresetButtonPressed(GtkButton *button);
        void OnDeletePresetButtonPressed(GtkButton *button);
        void OnDeleteAlertResponse(AdwAlertDialog *alert, gchar *response);

        void UpdatePreset(int presetRowIndex, const std::string &newName, bool updateValues)
        {
            auto presetId = m_DisplayedPresetNames[presetRowIndex];

            auto newValues = nlohmann::json{};
            if (updateValues)
            {
                auto preset = m_PresetPanelState->GetPreset(presetId);
                bool saveScannerSettings = preset->contains(PresetPanelState::k_ScannerSettingsKey);
                bool saveScanArea = preset->contains(PresetPanelState::k_ScanAreaKey);
                bool saveOutputSettings = preset->contains(PresetPanelState::k_OutputSettingsKey);

                GatherPresetValues(newValues, saveScannerSettings, saveScanArea, saveOutputSettings);
            }
            m_Dispatcher.Dispatch(UpdatePresetCommand(presetId, newName, newValues));
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions) override;
    };
}
