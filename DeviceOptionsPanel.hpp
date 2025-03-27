#pragma once

#include <gtk/gtk.h>
#include "ZooFW/CommandDispatcher.hpp"
#include "DeviceOptionState.hpp"
#include "ViewUpdateObserver.hpp"
#include "SaneDevice.hpp"
#include "App.hpp"

namespace ZooScan
{
    class DeviceOptionsPanel
    {
        App *m_App;

        const SaneDevice *m_Device;
        DeviceOptionState* m_DeviceOptions;

        Zoo::CommandDispatcher m_Dispatcher{};

        GtkWidget *m_RootWidget;
        GtkWidget *m_Viewport;

        ViewUpdateObserver<DeviceOptionsPanel, DeviceOptionState> *m_OptionUpdateObserver;

        std::unordered_map<uint64_t, GtkWidget*> m_Widgets;

        static std::string SaneIntOrFixedToString(int value, const DeviceOptionValueBase* option);
        static const char *SaneUnitToString(SANE_Unit unit);

        void BuildUI();

        static GtkWidget *AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *option);

        void AddCheckButton(GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);
        void AddVectorRow(GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex);
        void AddStringRow(GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);

        void OnCheckBoxChanged(GtkWidget *widget);
        void OnDropDownChanged(GtkWidget *widget);
        void OnNumericTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnStringTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnSpinButtonChanged(GtkWidget *widget);

    public:
        [[nodiscard]] const SaneDevice *Device() const
        { return m_Device; }

        [[nodiscard]] GtkWidget *RootWidget() const
        { return m_RootWidget; }

        DeviceOptionsPanel(const SaneDevice *saneDevice, Zoo::CommandDispatcher *parentDispatcher, App *app);

        ~DeviceOptionsPanel();

        void Update(const DeviceOptionState *stateComponent);
    };
}
