#pragma once
#include "Commands/DeleteScanItemCommand.hpp"
#include "Commands/LoadScanItemCommand.hpp"
#include "PresetPanel.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/View.hpp"

namespace Gorfector
{
    class ScanListPanel : public ZooLib::View
    {
        App *m_App;
        ZooLib::CommandDispatcher m_Dispatcher{};

        ScanListState *m_PanelState{};
        ViewUpdateObserver<ScanListPanel, ScanListState> *m_ViewUpdateObserver{};
        CurrentDeviceObserver<ScanListState> *m_CurrentDeviceObserver;

        std::vector<std::string> m_ScanListItemNames{};

        GtkWidget *m_RootWidget{};
        GtkWidget *m_ScanListButton{};
        GtkWidget *m_CancelListButton{};
        GtkWidget *m_ListBox{};

        void BuildUI();
        void OnScanClicked(GtkWidget *widget);

    public:
        ScanListPanel(ZooLib::CommandDispatcher *parentDispatcher, App *app)
            : m_App(app)
            , m_Dispatcher(parentDispatcher)
        {
            m_PanelState = new ScanListState(m_App->GetState());
            m_App->GetState()->LoadFromPreferenceFile(m_PanelState);

            BuildUI();

            m_ViewUpdateObserver = new ViewUpdateObserver(this, m_PanelState);
            m_App->GetObserverManager()->AddObserver(m_ViewUpdateObserver);

            m_CurrentDeviceObserver =
                    new CurrentDeviceObserver(m_App->GetAppState(), m_App->GetDeviceSelectorState(), m_PanelState);
            m_App->GetObserverManager()->AddObserver(m_CurrentDeviceObserver);

            m_Dispatcher.RegisterHandler(DeleteScanItemCommand::Execute, m_PanelState);
        }

        ~ScanListPanel() override
        {
            m_Dispatcher.UnregisterHandler<LoadScanItemCommand>();
            m_Dispatcher.UnregisterHandler<DeleteScanItemCommand>();

            m_App->GetObserverManager()->RemoveObserver(m_CurrentDeviceObserver);
            m_App->GetObserverManager()->RemoveObserver(m_ViewUpdateObserver);
            delete m_PanelState;
        }

        [[nodiscard]] ScanListState *GetState() const
        {
            return m_PanelState;
        }

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        GtkWidget *CreateScanListItem(const char *itemName);
        void OnLoadButtonPressed(GtkButton *button);
        void OnDeletePresetButtonPressed(GtkButton *button);
        void OnDeleteAlertResponse(AdwAlertDialog *alert, gchar *response);

        void Update(const std::vector<uint64_t> &lastSeenVersions) override;
    };
}
