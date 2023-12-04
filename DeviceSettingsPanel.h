#pragma once

#include <sane/sane.h>
#include <gtk/gtk.h>
#include "ZooFW/Application.h"
#include "ZooFW/CommandDispatcher.h"
#include "DeviceSettingsState.h"

namespace ZooScan
{

    class DeviceSettingsPanel
    {
        Zoo::Application *m_App;

        const SANE_Device *m_Device;

        Zoo::CommandDispatcher m_Dispatcher{};

        GtkWidget *m_RootWidget;

        DeviceSettingsState* m_Settings;

        static GtkWidget *AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor);

        void AddCheckbox(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex);
        void AddVectorRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex, int valueIndex);
        void AddStringRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex);

        void OnCheckBoxChanged(GtkWidget *widget);
        void OnDropDownChanged(GtkWidget *widget);
        void OnNumericTextFieldChanged(GtkWidget *widget);
        void OnStringTextFieldChanged(GtkWidget *widget);
        void OnSpinButtonChanged(GtkWidget *widget);

    public:
        [[nodiscard]] const SANE_Device *Device() const
        { return m_Device; }

        [[nodiscard]] GtkWidget *RootWidget() const
        { return m_RootWidget; }

        DeviceSettingsPanel(const SANE_Device *saneDevice, GtkBox *parentWidget, Zoo::CommandDispatcher *parentDispatche, Zoo::Application *app);

        ~DeviceSettingsPanel();
    };
}
