#include "DeviceSettingsPanel.h"
#include "ZooFW/ErrorDialog.h"
#include "ZooFW/SignalSupport.h"
#include "Commands/ChangeSettingCommand.h"

const char* SaneUnitToString(SANE_Unit unit)
{
    switch (unit)
    {
        case SANE_UNIT_NONE:
            return nullptr;
        case SANE_UNIT_PIXEL:
            return "px";
        case SANE_UNIT_BIT:
            return "bit";
        case SANE_UNIT_MM:
            return "mm";
        case SANE_UNIT_DPI:
            return "DPI";
        case SANE_UNIT_PERCENT:
            return "%%";
        case SANE_UNIT_MICROSECOND:
            return "ms";
        default:
            return nullptr;
    }
}

int EnsureBufferSize(void** buffer, int currentSize, int requestedSize)
{
    if (requestedSize > 1 << 24)
    {
        return currentSize;
    }

    if (currentSize < requestedSize)
    {
        free(*buffer);
        *buffer = malloc(requestedSize);
        return requestedSize;
    }

    return currentSize;
}

GtkWidget* ZooScan::DeviceSettingsPanel::AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor)
{
    auto settingBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent), settingBox);

    if (optionDescriptor->cap & SANE_CAP_ADVANCED)
    {
        gtk_widget_add_css_class(settingBox, "advanced");
    }

    return settingBox;
}

void ZooScan::DeviceSettingsPanel::AddCheckbox(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex)
{
    auto* checkBox = gtk_check_button_new_with_label(optionDescriptor->title);
    g_object_set_data(G_OBJECT(checkBox), "SettingIndex", GINT_TO_POINTER(settingIndex));
    gtk_box_append(GTK_BOX(parent), checkBox);
    Zoo::ConnectGtkSignal(this, &DeviceSettingsPanel::OnCheckBoxChanged, checkBox, "toggled");

    gtk_check_button_set_active(GTK_CHECK_BUTTON(checkBox), m_Settings->GetValue<bool>(settingIndex) != 0);

    if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
        gtk_widget_set_tooltip_text(checkBox, optionDescriptor->desc);

    if (!SANE_OPTION_IS_ACTIVE(optionDescriptor->cap) || !SANE_OPTION_IS_SETTABLE(optionDescriptor->cap))
    {
        gtk_widget_set_sensitive(checkBox, false);
    }
}

void ZooScan::DeviceSettingsPanel::AddVectorRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex, int valueIndex)
{
    if (valueIndex == 0)
    {
        auto *label = gtk_label_new(optionDescriptor->title);
        gtk_box_append(GTK_BOX(parent), label);

        if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
            gtk_widget_set_tooltip_text(label, optionDescriptor->desc);
    }

    int value = m_Settings->GetValue<int>(settingIndex, valueIndex);

    auto *elementBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent), elementBox);

    GtkWidget *valueWidget;

    if (optionDescriptor->constraint_type == SANE_CONSTRAINT_RANGE &&
        optionDescriptor->constraint.range != nullptr &&
        optionDescriptor->constraint.range->quant > 0)
    {
        valueWidget = gtk_spin_button_new_with_range(optionDescriptor->constraint.range->min,
                                                          optionDescriptor->constraint.range->max,
                                                          optionDescriptor->constraint.range->quant);
        gtk_box_append(GTK_BOX(elementBox), valueWidget);
        Zoo::ConnectGtkSignal(this, &DeviceSettingsPanel::OnSpinButtonChanged, valueWidget, "value-changed");

        double fieldValue = value;
        if (m_Settings->GetValueType(settingIndex) == SANE_TYPE_FIXED)
        {
            fieldValue = SANE_UNFIX(fieldValue);
        }
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(valueWidget), fieldValue);
    }
    else if (optionDescriptor->constraint_type == SANE_CONSTRAINT_WORD_LIST &&
             optionDescriptor->constraint.word_list != nullptr &&
             optionDescriptor->constraint.word_list[0] > 0)
    {
        auto *optionsStr = new std::string[optionDescriptor->constraint.word_list[0]];
        const char **options = new const char *[optionDescriptor->constraint.word_list[0] + 1];

        auto activeIndex = 0;
        for (auto wordIndex = 1; wordIndex < optionDescriptor->constraint.word_list[0] + 1; wordIndex++)
        {
            auto optionValue = optionDescriptor->constraint.word_list[wordIndex];

            if (optionValue == value)
            {
                activeIndex = wordIndex - 1;
            }

            optionsStr[wordIndex - 1] = m_Settings->GetValueType(settingIndex) == SANE_TYPE_FIXED ?
                    std::to_string(SANE_UNFIX(optionValue)) :
                    std::to_string(optionValue);
            options[wordIndex - 1] = optionsStr[wordIndex - 1].c_str();
        }
        options[optionDescriptor->constraint.word_list[0]] = nullptr;

        valueWidget = gtk_drop_down_new_from_strings(options);
        gtk_box_append(GTK_BOX(elementBox), valueWidget);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(valueWidget), activeIndex);
        Zoo::ConnectGtkSignalWithParamSpecs(this, &DeviceSettingsPanel::OnDropDownChanged, valueWidget, "notify::selected");

        delete[] options;
        delete[] optionsStr;
    }
    else // no constraint
    {
        valueWidget = gtk_entry_new();
        gtk_box_append(GTK_BOX(elementBox), valueWidget);
        Zoo::ConnectGtkSignal(this, &DeviceSettingsPanel::OnNumericTextFieldChanged, valueWidget, "changed");

        auto text = m_Settings->GetValueType(settingIndex) == SANE_TYPE_FIXED ? std::to_string(SANE_UNFIX(value))  : std::to_string(value);

        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(valueWidget));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(entryBuffer), text.c_str(),int(text.length()));
    }

    if (valueWidget != nullptr)
    {
        g_object_set_data(G_OBJECT(valueWidget), "SettingIndex", GINT_TO_POINTER(settingIndex));
    }

    if (valueWidget != nullptr && (!SANE_OPTION_IS_ACTIVE(optionDescriptor->cap) || !SANE_OPTION_IS_SETTABLE(optionDescriptor->cap)))
    {
        gtk_widget_set_sensitive(valueWidget, false);
    }

    // Add unit label
    if (optionDescriptor->unit != SANE_UNIT_NONE)
    {
        auto unitStr = SaneUnitToString(optionDescriptor->unit);
        if (unitStr != nullptr && strlen(unitStr) > 0)
        {
            auto *unitLabel = gtk_label_new(unitStr);
            gtk_box_append(GTK_BOX(elementBox), unitLabel);
        }
    }
}

void ZooScan::DeviceSettingsPanel::AddStringRow(GtkBox *parent, const SANE_Option_Descriptor *optionDescriptor, int settingIndex)
{
    auto *label = gtk_label_new(optionDescriptor->title);
    gtk_box_append(GTK_BOX(parent), label);

    if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
        gtk_widget_set_tooltip_text(label, optionDescriptor->desc);

    auto value = m_Settings->GetValue<std::string>(settingIndex);
    GtkWidget *valueWidget;

    if (optionDescriptor->constraint_type == SANE_CONSTRAINT_STRING_LIST &&
        optionDescriptor->constraint.string_list != nullptr)
    {
        auto activeIndex = 0;
        auto index = 0;
        while (optionDescriptor->constraint.string_list[index] != nullptr)
        {
            auto optionValue = optionDescriptor->constraint.string_list[index];

            if (strcmp(optionValue, value.c_str()) == 0)
            {
                activeIndex = index;
            }
            index++;
        }

        valueWidget = gtk_drop_down_new_from_strings(optionDescriptor->constraint.string_list);
        gtk_box_append(GTK_BOX(parent), valueWidget);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(valueWidget), activeIndex);
        Zoo::ConnectGtkSignalWithParamSpecs(this, &DeviceSettingsPanel::OnDropDownChanged, valueWidget, "notify::selected");
    }
    else // no constraint
    {
        valueWidget = gtk_entry_new();
        gtk_box_append(GTK_BOX(parent), valueWidget);
        Zoo::ConnectGtkSignal(this, &DeviceSettingsPanel::OnStringTextFieldChanged, valueWidget, "changed");

        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(valueWidget));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(entryBuffer), value.c_str(), int(value.length()));
    }

    if (valueWidget != nullptr)
    {
        g_object_set_data(G_OBJECT(valueWidget), "SettingIndex", GINT_TO_POINTER(settingIndex));
    }

    if (valueWidget != nullptr && (!SANE_OPTION_IS_ACTIVE(optionDescriptor->cap) || !SANE_OPTION_IS_SETTABLE(optionDescriptor->cap)))
    {
        gtk_widget_set_sensitive(valueWidget, false);
    }
}

ZooScan::DeviceSettingsPanel::DeviceSettingsPanel(const SANE_Device *saneDevice, GtkBox *parentWidget, Zoo::CommandDispatcher *parentDispatcher,
                                                  Zoo::Application *app)
        : m_Device(saneDevice)
        , m_Dispatcher(parentDispatcher)
        , m_App(app)
{
    if (m_Device == nullptr)
        return;

    SANE_Status status;
    SANE_Handle handle;

    status = sane_open(m_Device->name, &handle);
    if (status != SANE_STATUS_GOOD)
    {
        auto parentWindow = gtk_widget_get_root(GTK_WIDGET(parentWidget));
        Zoo::ShowUserError(GTK_WINDOW(parentWindow), "Failed to open device");
        return;
    }

    m_Settings = new DeviceSettingsState();
    m_App->GetState()->AddStateComponent(m_Settings);

    DeviceSettingsState::Updater settingsUpdater(m_Settings);

    m_RootWidget = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_RootWidget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(m_RootWidget), TRUE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(m_RootWidget), TRUE);
    auto viewport = gtk_viewport_new(gtk_adjustment_new(0, 0, 0, 0, 0, 0), gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_RootWidget), viewport);
    auto scrollViewContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_viewport_set_child(GTK_VIEWPORT(viewport), scrollViewContent);
    gtk_widget_add_css_class(scrollViewContent, "settings-content");

    auto* topLevel = scrollViewContent;
    auto* currentParent = topLevel;

    int valueBufferSize = sizeof(SANE_Word) * 512;
    void* value = malloc(valueBufferSize);
    int optionCount = 1;
    const SANE_Option_Descriptor *optionDescriptor = sane_get_option_descriptor(handle, optionCount);
    while (optionDescriptor != nullptr)
    {
        g_print("Option %d: %s\n", optionCount, optionDescriptor->title);

        auto displayOption = true;
        if (optionDescriptor->type != SANE_TYPE_GROUP && optionDescriptor->type != SANE_TYPE_BUTTON)
        {
            valueBufferSize = EnsureBufferSize(&value, valueBufferSize, optionDescriptor->size);
            if (valueBufferSize < optionDescriptor->size)
            {
                auto parentWindow = gtk_widget_get_root(GTK_WIDGET(parentWidget));
                Zoo::ShowUserError(GTK_WINDOW(parentWindow), "Failed to allocate memory for option: %s\n", optionDescriptor->title);
                displayOption = false;
            }
            else if (sane_control_option(handle, optionCount, SANE_ACTION_GET_VALUE, value, nullptr) != SANE_STATUS_GOOD)
            {
                auto parentWindow = gtk_widget_get_root(GTK_WIDGET(parentWidget));
                Zoo::ShowUserError(GTK_WINDOW(parentWindow), "Failed to get option: %s\n", optionDescriptor->title);
                displayOption = false;
            }
        }

        if (displayOption)
        {
            switch (optionDescriptor->type)
            {
                case SANE_TYPE_BOOL:
                {
                    auto *boolValue = new DeviceSettingValue<bool>(optionCount - 1, optionDescriptor->type);
                    settingsUpdater.AddSettingValue(optionCount - 1, boolValue);
                    settingsUpdater.SetValue(optionCount - 1, *static_cast<SANE_Bool *>(value) != 0);

                    auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);
                    AddCheckbox(GTK_BOX(settingBox), optionDescriptor, optionCount - 1);
                    break;
                }

                case SANE_TYPE_INT:
                case SANE_TYPE_FIXED:
                {
                    int numberOfElements = int(optionDescriptor->size / sizeof(SANE_Word));
                    if (numberOfElements > 4)
                        break;

                    auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);

                    GtkWidget *vectorBox;
                    if (numberOfElements == 1)
                    {
                        vectorBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                        gtk_box_append(GTK_BOX(settingBox), vectorBox);
                    }
                    else
                    {
                        vectorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                        gtk_box_append(GTK_BOX(settingBox), vectorBox);
                    }

                    auto *intValue = new DeviceSettingValue<int>(optionCount - 1,optionDescriptor->type, numberOfElements);
                    settingsUpdater.AddSettingValue(optionCount - 1, intValue);
                    for (auto i = 0; i < numberOfElements; i++)
                    {
                        settingsUpdater.SetValue(optionCount - 1, i, static_cast<SANE_Int *>(value)[i]);
                        AddVectorRow(GTK_BOX(vectorBox), optionDescriptor,  optionCount - 1, i);
                    }
                    break;
                }

                case SANE_TYPE_STRING:
                {
                    auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);

                    auto *strValue = new DeviceSettingValue<std::string>(optionCount - 1,optionDescriptor->type);
                    settingsUpdater.AddSettingValue(optionCount - 1, strValue);
                    settingsUpdater.SetValue(optionCount - 1,std::string(static_cast<SANE_String>(value)));
                    AddStringRow(GTK_BOX(settingBox), optionDescriptor,  optionCount - 1);
                    break;
                }

                case SANE_TYPE_GROUP:
                {
                    auto *frame = gtk_frame_new(optionDescriptor->title);
                    gtk_box_append(GTK_BOX(topLevel), frame);
                    auto *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                    gtk_frame_set_child(GTK_FRAME(frame), box);

                    gtk_widget_add_css_class(frame, "settings-group");
                    if (optionDescriptor->cap & SANE_CAP_ADVANCED)
                    {
                        //gtk_widget_add_css_class(frame, "advanced");
                    }

                    if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
                        gtk_widget_set_tooltip_text(frame, optionDescriptor->desc);

                    currentParent = box;
                    break;
                }

                case SANE_TYPE_BUTTON:
                {
                    auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);
                    auto *button = gtk_button_new_with_label(optionDescriptor->title);
                    gtk_box_append(GTK_BOX(settingBox), button);

                    if (optionDescriptor->cap & SANE_CAP_ADVANCED)
                    {
                        gtk_widget_add_css_class(button, "advanced");
                    }

                    if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
                        gtk_widget_set_tooltip_text(button, optionDescriptor->desc);

                    break;
                }

                default:
                    break;
            }
        }

        optionCount++;
        optionDescriptor = sane_get_option_descriptor(handle, optionCount);
    }

    free(value);

    sane_close(handle);

    m_Dispatcher.RegisterHandler<ChangeSettingCommand<bool>, DeviceSettingsState>(ChangeSettingCommand<bool>::Execute, m_Settings);
    m_Dispatcher.RegisterHandler<ChangeSettingCommand<int>, DeviceSettingsState>(ChangeSettingCommand<int>::Execute, m_Settings);
    m_Dispatcher.RegisterHandler<ChangeSettingCommand<std::string>, DeviceSettingsState>(ChangeSettingCommand<std::string>::Execute, m_Settings);
}

ZooScan::DeviceSettingsPanel::~DeviceSettingsPanel()
{
    m_Dispatcher.UnregisterHandler<ChangeSettingCommand<bool>>();
    m_Dispatcher.UnregisterHandler<ChangeSettingCommand<int>>();
    m_Dispatcher.UnregisterHandler<ChangeSettingCommand<std::string>>();

    m_App->GetState()->RemoveStateComponent(m_Settings);
    delete m_Settings;
}

void ZooScan::DeviceSettingsPanel::OnCheckBoxChanged(GtkWidget* widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "SettingIndex");
    if (gObjectData == nullptr)
        return;

    auto index = GPOINTER_TO_INT(gObjectData);

    auto *checkBox = GTK_CHECK_BUTTON(widget);
    auto isChecked = gtk_check_button_get_active(checkBox);

    m_Dispatcher.Dispatch(ChangeSettingCommand<bool>(index, isChecked));
}

void ZooScan::DeviceSettingsPanel::OnDropDownChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "SettingIndex");
    if (gObjectData == nullptr)
        return;

    auto index = GPOINTER_TO_INT(gObjectData);
    auto saneType = m_Settings->GetValueType(index);

    auto *dropDown = GTK_DROP_DOWN(widget);
    auto selectedItem = G_OBJECT(gtk_drop_down_get_selected_item(dropDown));
    if (saneType == SANE_TYPE_FIXED)
    {
        // FIXME: drop down item should have the real value so we don't need to parse it
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        auto value = std::stod(itemName);
        auto fixedValue = SANE_FIX(value);
        m_Dispatcher.Dispatch(ChangeSettingCommand<int>(index, fixedValue));
    }
    else if (saneType == SANE_TYPE_INT)
    {
        // FIXME: drop down item should have the real value so we don't need to parse it
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        auto value = std::stoi(itemName);
        m_Dispatcher.Dispatch(ChangeSettingCommand<int>(index, value));
    }
    else if (saneType == SANE_TYPE_STRING)
    {
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        m_Dispatcher.Dispatch(ChangeSettingCommand<std::string>(index, itemName));
    }
}

void ZooScan::DeviceSettingsPanel::OnNumericTextFieldChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "SettingIndex");
    if (gObjectData == nullptr)
        return;

    auto index = GPOINTER_TO_INT(gObjectData);
    auto saneType = m_Settings->GetValueType(index);

    if ((saneType == SANE_TYPE_FIXED) || (saneType == SANE_TYPE_INT))
    {
        auto *textField = GTK_ENTRY(widget);
        auto *entryBuffer = gtk_entry_get_buffer(textField);
        auto text = gtk_entry_buffer_get_text(entryBuffer);
        int value;
        if (saneType == SANE_TYPE_FIXED)
        {
            double v = std::stod(text);
            value = SANE_FIX(v);
        }
        else
        {
            value = std::stoi(text);
        }

        // FIXME: value index
        m_Dispatcher.Dispatch(ChangeSettingCommand<int>(index, value));
    }
}

void ZooScan::DeviceSettingsPanel::OnStringTextFieldChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "SettingIndex");
    if (gObjectData == nullptr)
        return;

    auto index = GPOINTER_TO_INT(gObjectData);
    auto saneType = m_Settings->GetValueType(index);

    if (saneType == SANE_TYPE_STRING)
    {
        auto *textField = GTK_ENTRY(widget);
        auto *entryBuffer = gtk_entry_get_buffer(textField);
        auto text = gtk_entry_buffer_get_text(entryBuffer);

        m_Dispatcher.Dispatch(ChangeSettingCommand<std::string>(index, text));
    }
}

void ZooScan::DeviceSettingsPanel::OnSpinButtonChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "SettingIndex");
    if (gObjectData == nullptr)
        return;

    auto index = GPOINTER_TO_INT(gObjectData);
    auto saneType = m_Settings->GetValueType(index);

    if ((saneType == SANE_TYPE_FIXED) || (saneType == SANE_TYPE_INT))
    {
        auto *spinButton = GTK_SPIN_BUTTON(widget);
        auto fieldValue = gtk_spin_button_get_value(spinButton);
        int value;
        if (saneType == SANE_TYPE_FIXED)
        {
            value = SANE_FIX(fieldValue);
        }
        else
        {
            value = int(fieldValue);
        }

        m_Dispatcher.Dispatch(ChangeSettingCommand<int>(index, value));
    }
}
