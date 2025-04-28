#pragma once

#include "App.hpp"
#include "Commands/ApplyPresetCommand.hpp"
#include "Commands/CreatePresetCommand.hpp"
#include "Commands/DeletePresetCommand.hpp"
#include "Commands/RenamePresetCommand.hpp"
#include "Commands/SetPresetExpanded.hpp"
#include "CurrentDeviceObserver.hpp"
#include "PresetPanelDialogs.hpp"
#include "PresetPanelState.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/GtkUtils.hpp"
#include "ZooLib/View.hpp"

namespace ZooScan
{
    GtkWidget *CreatePresetListItem(gpointer item, gpointer userData);

    class PresetPanel : public ZooLib::View
    {
        App *m_App;
        ZooLib::CommandDispatcher m_Dispatcher{};

        PresetPanelState *m_PresetPanelState{};
        ViewUpdateObserver<PresetPanel, PresetPanelState> *m_ViewUpdateObserver{};
        CurrentDeviceObserver *m_CurrentDeviceObserver;

        std::string m_CurrentDeviceModel{};
        std::string m_CurrentDeviceVendor{};

        GtkWidget *m_RootWidget{};
        GtkWidget *m_Expander{};
        GtkWidget *m_CreatePresetButton{};
        GtkWidget *m_ListBox{};
        std::vector<std::string> m_DisplayedPresetNames{};

        void OnExpanderExpanded(GtkWidget *widget)
        {
            m_Dispatcher.Dispatch(SetPresetExpanded(gtk_expander_get_expanded(GTK_EXPANDER(widget))));
        }

    public:
        PresetPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app)
            : m_App(app)
            , m_Dispatcher(parentDispatcher)
        {
            m_PresetPanelState = new PresetPanelState(m_App->GetState());
            m_App->GetState()->LoadFromPreferenceFile(m_PresetPanelState);
            BuildUI();

            m_ViewUpdateObserver = new ViewUpdateObserver(this, m_PresetPanelState);
            m_App->GetObserverManager()->AddObserver(m_ViewUpdateObserver);

            m_CurrentDeviceObserver = new CurrentDeviceObserver(
                    m_App->GetAppState(), m_App->GetDeviceSelectorState(), m_PresetPanelState);
            m_App->GetObserverManager()->AddObserver(m_CurrentDeviceObserver);

            m_Dispatcher.RegisterHandler(SetPresetExpanded::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(CreatePresetCommand::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(DeletePresetCommand::Execute, m_PresetPanelState);
            m_Dispatcher.RegisterHandler(RenamePresetCommand::Execute, m_PresetPanelState);
        }

        ~PresetPanel() override
        {
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

        bool IsUniquePresetName(std::string &presetName) const
        {
            return m_PresetPanelState->GetPreset(presetName) == nullptr;
        }

        void CreatePreset(
                const std::string &presetName, bool saveScannerSettings, bool saveScanArea, bool saveOutputSettings)
        {
            auto preset = nlohmann::json{};
            preset["Name"] = presetName;

            auto scannerSettings = nlohmann::json{};
            to_json(scannerSettings, *m_App->GetDeviceOptions());

            if (saveScanArea)
            {
                preset[PresetPanelState::s_PresetSectionScanArea] = {
                        {"tl-x", scannerSettings["Options"]["tl-x"]},
                        {"tl-y", scannerSettings["Options"]["tl-y"]},
                        {"br-x", scannerSettings["Options"]["br-x"]},
                        {"br-y", scannerSettings["Options"]["br-y"]},
                };
            }
            if (saveScannerSettings)
            {
                preset[PresetPanelState::s_PresetSectionScannerSettings] = scannerSettings;

                preset[PresetPanelState::s_PresetSectionScannerSettings]["Options"].erase("tl-x");
                preset[PresetPanelState::s_PresetSectionScannerSettings]["Options"].erase("tl-y");
                preset[PresetPanelState::s_PresetSectionScannerSettings]["Options"].erase("br-x");
                preset[PresetPanelState::s_PresetSectionScannerSettings]["Options"].erase("br-y");
            }
            if (saveOutputSettings)
            {
                preset["OutputSettings"] = {};
                to_json(preset[PresetPanelState::s_PresetSectionOutputSettings], *m_App->GetOutputOptions());
            }

            m_Dispatcher.Dispatch(CreatePresetCommand(preset));
        }

        [[nodiscard]] const nlohmann::json *GetPreset(int presetRowIndex) const
        {
            auto presetName = m_DisplayedPresetNames[presetRowIndex];
            return m_PresetPanelState->GetPreset(presetName);
        }

        void OnApplyPresetButtonPressed(GtkButton *button);
        void OnDeletePresetButtonPressed(GtkButton *button);
        void OnDeleteAlertResponse(AdwAlertDialog *alert, gchar *response);

        void RenamePreset(int presetRowIndex, const std::string &newName)
        {
            auto presetName = m_DisplayedPresetNames[presetRowIndex];
            m_Dispatcher.Dispatch(RenamePresetCommand(presetName, newName));
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions) override
        {
            gtk_expander_set_expanded(GTK_EXPANDER(m_Expander), m_PresetPanelState->IsExpanded());

            m_CurrentDeviceModel = m_PresetPanelState->GetCurrentDeviceModel();
            m_CurrentDeviceVendor = m_PresetPanelState->GetCurrentDeviceVendor();

            m_Dispatcher.UnregisterHandler<ApplyPresetCommand>();
            m_Dispatcher.RegisterHandler(
                    ApplyPresetCommand::Execute, m_App->GetDeviceOptions(), m_App->GetOutputOptions());

            bool canCreateOrApplyPreset = m_PresetPanelState->CanCreateOrApplyPreset();
            gtk_widget_set_sensitive(m_CreatePresetButton, canCreateOrApplyPreset);

            m_DisplayedPresetNames.clear();
            for (const auto &preset: m_PresetPanelState->GetPresetsForScanner(
                         m_PresetPanelState->GetCurrentDeviceVendor(), m_PresetPanelState->GetCurrentDeviceModel()))
            {
                m_DisplayedPresetNames.push_back(preset["Name"].get<std::string>());
            }

            const char **names = new const char *[m_DisplayedPresetNames.size() + 1];
            for (size_t i = 0; i < m_DisplayedPresetNames.size(); ++i)
            {
                names[i] = m_DisplayedPresetNames[i].c_str();
            }
            names[m_DisplayedPresetNames.size()] = nullptr;

            auto presetNamesList = gtk_string_list_new(names);
            gtk_list_box_bind_model(
                    GTK_LIST_BOX(m_ListBox), G_LIST_MODEL(presetNamesList), ZooScan::CreatePresetListItem, this,
                    nullptr);

            int i = 0;
            auto row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(m_ListBox), i);
            while (row != nullptr)
            {
                auto applyButton = ZooLib::FindWidgetByName(GTK_WIDGET(row), "apply-button");
                if (applyButton != nullptr)
                {
                    gtk_widget_set_sensitive(applyButton, canCreateOrApplyPreset);
                }

                row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(m_ListBox), ++i);
            }
        }
    };
}
