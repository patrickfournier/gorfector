#pragma once

#include <gtk/gtk.h>

#include "App.hpp"
#include "DeviceOptionsState.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace ZooScan
{
    class DeviceOptionsPanel : public ZooLib::View
    {
        App *m_App;

        const int m_SaneInitId;
        const std::string m_DeviceName;
        DeviceOptionsState *m_DeviceOptions;

        ZooLib::CommandDispatcher m_Dispatcher;

        GtkWidget *m_RootWidget{};
        GtkWidget *m_OptionParent;
        GtkWidget *m_PageBasic{};
        GtkWidget *m_PageAdvanced{};

        ViewUpdateObserver<DeviceOptionsPanel, DeviceOptionsState> *m_OptionUpdateObserver;

        std::unordered_map<uint64_t, GtkWidget *> m_Widgets;

        static std::string SaneIntOrFixedToString(int value, const DeviceOptionValueBase *option);
        static const char *SaneUnitToString(SANE_Unit unit);

        void AddOtherOptions();

        static GtkWidget *AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *option);

        void AddCheckButton(GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);
        void AddVectorRow(
                GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex,
                bool multiValue);
        void AddStringRow(GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);

        void OnCheckBoxChanged(GtkWidget *widget);
        void OnDropDownChanged(GtkWidget *widget);
        void OnNumericTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnStringTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnSpinButtonChanged(GtkWidget *widget);

    public:
        [[nodiscard]] int GetSaneInitId() const
        {
            return m_SaneInitId;
        }

        [[nodiscard]] const std::string &GetDeviceName() const
        {
            return m_DeviceName;
        }

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        [[nodiscard]] DeviceOptionsState *GetState() const
        {
            return m_DeviceOptions;
        }

        DeviceOptionsPanel(
                int saneInitId, std::string deviceName, ZooLib::CommandDispatcher *parentDispatcher, App *app);

        ~DeviceOptionsPanel() override;
        void BuildUI();
        void AddCommonOptions();
        GtkWidget *
        AddOptionRow(uint64_t optionIndex, GtkWidget *parent, bool skipBasicOptions, bool skipAdvancedOptions);

        void Update(const std::vector<uint64_t> &lastSeenVersions) override;
    };
}
