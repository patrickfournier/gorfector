#pragma once

#include <gtk/gtk.h>
#include "ZooFW/Application.h"
#include "ZooFW/CommandDispatcher.h"
#include "DeviceSettingsState.h"
#include "ViewUpdateObserver.h"
#include "SaneDevice.h"

namespace ZooScan
{

    class DeviceSettingsPanel
    {
        Zoo::Application *m_App;

        const SaneDevice *m_Device;

        Zoo::CommandDispatcher m_Dispatcher{};

        GtkWidget *m_RootWidget;
        GtkWidget *m_Viewport;

        DeviceSettingsState* m_Settings;
        ViewUpdateObserver<DeviceSettingsPanel, DeviceSettingsState> *m_SettingUpdateObserver;

        std::unordered_map<uint64_t, GtkWidget*> m_Widgets;

        void BuildUI();

        static GtkWidget *AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor);

        void AddCheckButton(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex);
        void AddVectorRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex, int valueIndex);
        void AddStringRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex);

        void OnCheckBoxChanged(GtkWidget *widget);
        void OnDropDownChanged(GtkWidget *widget);
        void OnNumericTextFieldChanged(GtkWidget *widget);
        void OnStringTextFieldChanged(GtkWidget *widget);
        void OnSpinButtonChanged(GtkWidget *widget);

    public:
        [[nodiscard]] const SaneDevice *Device() const
        { return m_Device; }

        [[nodiscard]] GtkWidget *RootWidget() const
        { return m_RootWidget; }

        DeviceSettingsPanel(const SaneDevice *saneDevice, Zoo::CommandDispatcher *parentDispatcher, Zoo::Application *app);

        ~DeviceSettingsPanel();

        void Update(DeviceSettingsState *stateComponent);
    };
}
