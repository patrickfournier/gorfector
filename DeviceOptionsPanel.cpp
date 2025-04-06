#include "DeviceOptionsPanel.hpp"
#include <memory>
#include "Commands/ChangeOptionCommand.hpp"
#include "SaneDevice.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/ErrorDialog.hpp"
#include "ZooLib/SignalSupport.hpp"


const char *ZooScan::DeviceOptionsPanel::SaneUnitToString(SANE_Unit unit)
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

std::string ZooScan::DeviceOptionsPanel::SaneIntOrFixedToString(int value, const DeviceOptionValueBase *option)
{
    if (option->GetValueType() == SANE_TYPE_FIXED)
    {
        if (option->GetUnit() == SANE_UNIT_MM)
        {
            char strBuffer[32];
            std::sprintf(strBuffer, "%.2f", SANE_UNFIX(value));
            return strBuffer;
        }
        else
        {
            return std::to_string(SANE_UNFIX(value));
        }
    }
    else
    {
        return std::to_string(value);
    }
}

GtkWidget *ZooScan::DeviceOptionsPanel::AddSettingBox(GtkBox *parent, const SANE_Option_Descriptor *option)
{
    auto settingBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent), settingBox);

    if (DeviceOptionValueBase::ShouldHide(*option))
    {
        gtk_widget_set_visible(settingBox, false);
    }

    if (DeviceOptionValueBase::IsAdvanced(*option))
    {
        gtk_widget_add_css_class(settingBox, "advanced");
    }

    return settingBox;
}

void ZooScan::DeviceOptionsPanel::AddCheckButton(
        GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex)
{
    auto boolOption = dynamic_cast<const DeviceOptionValue<bool> *>(option);
    if (boolOption == nullptr)
        return;

    if (option->IsDisplayOnly())
    {
        auto *label = gtk_label_new(option->GetTitle());
        gtk_box_append(GTK_BOX(parent), label);
        if (option->GetDescription() != nullptr && strlen(option->GetDescription()) > 0)
            gtk_widget_set_tooltip_text(label, option->GetDescription());

        label = gtk_label_new(boolOption->GetValue() ? "Yes" : "No");
        gtk_box_append(GTK_BOX(parent), label);

        return;
    }

    auto *checkButton = gtk_check_button_new_with_label(option->GetTitle());
    gtk_box_append(GTK_BOX(parent), checkButton);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(checkButton), boolOption->GetValue() != 0);

    if (option->GetDescription() != nullptr && strlen(option->GetDescription()) > 0)
        gtk_widget_set_tooltip_text(checkButton, option->GetDescription());

    auto index = WidgetIndex(settingIndex, 0);
    g_object_set_data(G_OBJECT(checkButton), "OptionIndex", GINT_TO_POINTER(index.Hash()));
    m_Widgets[index.Hash()] = checkButton;

    ZooLib::ConnectGtkSignal(this, &DeviceOptionsPanel::OnCheckBoxChanged, checkButton, "toggled");
}

void ZooScan::DeviceOptionsPanel::AddVectorRow(
        GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex)
{
    auto intOption = dynamic_cast<const DeviceOptionValue<int> *>(option);
    if (intOption == nullptr)
        return;

    if (valueIndex == 0)
    {
        auto *label = gtk_label_new(option->GetTitle());
        gtk_box_append(GTK_BOX(parent), label);

        if (option->GetDescription() != nullptr && strlen(option->GetDescription()) > 0)
            gtk_widget_set_tooltip_text(label, option->GetDescription());
    }

    int value = intOption->GetValue(valueIndex);

    auto *elementBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent), elementBox);

    GtkWidget *valueWidget = nullptr;

    if (option->IsDisplayOnly())
    {
        auto valueStr = SaneIntOrFixedToString(value, option);
        auto label = gtk_label_new(valueStr.c_str());
        gtk_box_append(GTK_BOX(elementBox), label);
    }
    else if (auto range = option->GetRange(); range != nullptr && range->quant != 0)
    {
        valueWidget = gtk_spin_button_new_with_range(range->min, range->max, range->quant);
        gtk_box_append(GTK_BOX(elementBox), valueWidget);

        double fieldValue = value;
        if (option->GetValueType() == SANE_TYPE_FIXED)
        {
            fieldValue = SANE_UNFIX(fieldValue);
        }
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(valueWidget), fieldValue);

        ZooLib::ConnectGtkSignal(this, &DeviceOptionsPanel::OnSpinButtonChanged, valueWidget, "value-changed");
    }
    else if (auto wordList = option->GetNumberList(); wordList != nullptr)
    {
        std::unique_ptr<std::string[]> optionsStr(new std::string[wordList[0]]); // storage for option strings
        std::unique_ptr<const char *[]> options(new const char *[wordList[0] + 1]); // list of char* for drop down

        auto activeIndex = 0;
        for (auto wordIndex = 1; wordIndex < wordList[0] + 1; wordIndex++)
        {
            auto optionValue = wordList[wordIndex];

            if (optionValue == value)
            {
                activeIndex = wordIndex - 1;
            }

            optionsStr[wordIndex - 1] = option->GetValueType() == SANE_TYPE_FIXED
                                                ? std::to_string(SANE_UNFIX(optionValue))
                                                : std::to_string(optionValue);
            options[wordIndex - 1] = optionsStr[wordIndex - 1].c_str();
        }
        options[wordList[0]] = nullptr;

        valueWidget = gtk_drop_down_new_from_strings(options.get());
        gtk_box_append(GTK_BOX(elementBox), valueWidget);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(valueWidget), activeIndex);

        ZooLib::ConnectGtkSignalWithParamSpecs(
                this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget, "notify::selected");
    }
    else // no constraint
    {
        valueWidget = gtk_entry_new();
        auto focusController = gtk_event_controller_focus_new();
        gtk_widget_add_controller(valueWidget, focusController);
        gtk_box_append(GTK_BOX(elementBox), valueWidget);

        std::string text = SaneIntOrFixedToString(value, option);
        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(valueWidget));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(entryBuffer), text.c_str(), static_cast<int>(text.length()));

        ZooLib::ConnectGtkSignal(this, &DeviceOptionsPanel::OnNumericTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        auto index = WidgetIndex(settingIndex, valueIndex);
        g_object_set_data(G_OBJECT(valueWidget), "OptionIndex", GINT_TO_POINTER(index.Hash()));
        m_Widgets[index.Hash()] = valueWidget;
    }

    // Add unit label
    if (auto unitStr = SaneUnitToString(option->GetUnit()); unitStr != nullptr && strlen(unitStr) > 0)
    {
        auto *unitLabel = gtk_label_new(unitStr);
        gtk_box_append(GTK_BOX(elementBox), unitLabel);
    }
}

void ZooScan::DeviceOptionsPanel::AddStringRow(
        GtkBox *parent, const DeviceOptionValueBase *option, uint32_t settingIndex)
{
    auto strOption = dynamic_cast<const DeviceOptionValue<std::string> *>(option);
    if (strOption == nullptr)
        return;

    auto *label = gtk_label_new(option->GetTitle());
    gtk_box_append(GTK_BOX(parent), label);

    if (option->GetDescription() != nullptr && strlen(option->GetDescription()) > 0)
        gtk_widget_set_tooltip_text(label, option->GetDescription());

    auto value = strOption->GetValue();
    GtkWidget *valueWidget = nullptr;

    if (option->IsDisplayOnly())
    {
        auto valueLabel = gtk_label_new(value.c_str());
        gtk_box_append(GTK_BOX(parent), valueLabel);
    }
    else if (auto stringList = option->GetStringList(); stringList != nullptr)
    {
        auto activeIndex = 0;
        auto index = 0;
        while (stringList[index] != nullptr)
        {
            auto optionValue = stringList[index];

            if (strcmp(optionValue, value.c_str()) == 0)
            {
                activeIndex = index;
            }
            index++;
        }

        valueWidget = gtk_drop_down_new_from_strings(stringList);
        gtk_box_append(GTK_BOX(parent), valueWidget);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(valueWidget), activeIndex);

        ZooLib::ConnectGtkSignalWithParamSpecs(
                this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget, "notify::selected");
    }
    else // no constraint
    {
        valueWidget = gtk_entry_new();
        auto focusController = gtk_event_controller_focus_new();
        gtk_widget_add_controller(valueWidget, focusController);
        gtk_box_append(GTK_BOX(parent), valueWidget);

        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(valueWidget));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(entryBuffer), value.c_str(), static_cast<int>(value.length()));

        ZooLib::ConnectGtkSignal(this, &DeviceOptionsPanel::OnStringTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        auto index = WidgetIndex(settingIndex, 0);
        g_object_set_data(G_OBJECT(valueWidget), "OptionIndex", GINT_TO_POINTER(index.Hash()));
        m_Widgets[index.Hash()] = valueWidget;
    }
}

ZooScan::DeviceOptionsPanel::DeviceOptionsPanel(
        int saneInitId, const std::string &deviceName, ZooLib::CommandDispatcher *parentDispatcher, App *app)
    : m_App(app)
    , m_SaneInitId(saneInitId)
    , m_DeviceName(deviceName)
    , m_Dispatcher(parentDispatcher)
{
    if (m_DeviceName.empty())
        return;

    m_DeviceOptions = new DeviceOptionsState(m_App->GetState(), m_DeviceName);

    m_OptionUpdateObserver = new ViewUpdateObserver(this, m_DeviceOptions);
    m_App->GetObserverManager()->AddObserver(m_OptionUpdateObserver);

    m_RootWidget = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_RootWidget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(m_RootWidget), TRUE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(m_RootWidget), TRUE);
    m_Viewport = gtk_viewport_new(gtk_adjustment_new(0, 0, 0, 0, 0, 0), gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_RootWidget), m_Viewport);

    BuildUI();

    m_Dispatcher.RegisterHandler<ChangeOptionCommand<bool>, DeviceOptionsState>(
            ChangeOptionCommand<bool>::Execute, m_DeviceOptions);
    m_Dispatcher.RegisterHandler<ChangeOptionCommand<int>, DeviceOptionsState>(
            ChangeOptionCommand<int>::Execute, m_DeviceOptions);
    m_Dispatcher.RegisterHandler<ChangeOptionCommand<std::string>, DeviceOptionsState>(
            ChangeOptionCommand<std::string>::Execute, m_DeviceOptions);
}

ZooScan::DeviceOptionsPanel::~DeviceOptionsPanel()
{
    m_Dispatcher.UnregisterHandler<ChangeOptionCommand<bool>>();
    m_Dispatcher.UnregisterHandler<ChangeOptionCommand<int>>();
    m_Dispatcher.UnregisterHandler<ChangeOptionCommand<std::string>>();

    m_App->GetObserverManager()->RemoveObserver(m_OptionUpdateObserver);
    delete m_OptionUpdateObserver;

    delete m_DeviceOptions;
}

void ZooScan::DeviceOptionsPanel::BuildUI()
{
    m_Widgets.clear();

    auto scrollViewContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_viewport_set_child(GTK_VIEWPORT(m_Viewport), scrollViewContent);
    gtk_widget_add_css_class(scrollViewContent, "settings-content");

    auto *topLevel = scrollViewContent;
    auto *currentParent = topLevel;

    for (auto optionIndex = 0UL; optionIndex < m_DeviceOptions->GetOptionCount(); optionIndex++)
    {
        auto device = m_App->GetDeviceByName(m_DeviceName);
        auto optionDescriptor = device->GetOptionDescriptor(optionIndex);
        const auto *optionValue = m_DeviceOptions->GetOption(optionIndex);
        if (optionDescriptor == nullptr)
        {
            continue;
        }

        switch (optionDescriptor->type)
        {
            case SANE_TYPE_BOOL:
            {
                if (optionValue == nullptr)
                    continue;

                auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);
                AddCheckButton(GTK_BOX(settingBox), optionValue, optionIndex);
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                if (optionValue == nullptr)
                    continue;

                auto numberOfElements = optionValue->GetValueCount();
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

                for (auto i = 0U; i < numberOfElements; i++)
                {
                    AddVectorRow(GTK_BOX(vectorBox), optionValue, optionIndex, i);
                }
                break;
            }

            case SANE_TYPE_STRING:
            {
                if (optionValue == nullptr)
                    continue;

                auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);
                AddStringRow(GTK_BOX(settingBox), optionValue, optionIndex);
                break;
            }

            case SANE_TYPE_GROUP:
            {
                auto *frame = gtk_frame_new(optionDescriptor->title);
                gtk_box_append(GTK_BOX(topLevel), frame);
                auto *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                gtk_frame_set_child(GTK_FRAME(frame), box);

                gtk_widget_add_css_class(frame, "settings-group");
                if (DeviceOptionValueBase::IsAdvanced(*optionDescriptor))
                {
                    // gtk_widget_add_css_class(frame, "advanced");
                }

                if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
                    gtk_widget_set_tooltip_text(frame, optionDescriptor->desc);

                if (DeviceOptionValueBase::ShouldHide(*optionDescriptor))
                {
                    gtk_widget_set_visible(frame, false);
                }

                currentParent = box;
                break;
            }

            case SANE_TYPE_BUTTON:
            {
                auto *settingBox = AddSettingBox(GTK_BOX(currentParent), optionDescriptor);
                auto *button = gtk_button_new_with_label(optionDescriptor->title);
                gtk_box_append(GTK_BOX(settingBox), button);

                if (DeviceOptionValueBase::IsAdvanced(*optionDescriptor))
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
}

void ZooScan::DeviceOptionsPanel::OnCheckBoxChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto *checkBox = GTK_CHECK_BUTTON(widget);
    auto isChecked = gtk_check_button_get_active(checkBox);

    try
    {
        m_Dispatcher.Dispatch(ChangeOptionCommand<bool>(optionIndex, valueIndex, isChecked));
    }
    catch (std::runtime_error &e)
    {
        ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
    }
}

void ZooScan::DeviceOptionsPanel::OnDropDownChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto option = m_DeviceOptions->GetOption(optionIndex);
    auto saneType = option->GetValueType();

    auto *dropDown = GTK_DROP_DOWN(widget);
    auto selectedItem = G_OBJECT(gtk_drop_down_get_selected_item(dropDown));
    if (saneType == SANE_TYPE_FIXED)
    {
        // FIXME: drop down item should have the real value so we don't need to parse it
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        auto value = std::stod(itemName);
        auto fixedValue = SANE_FIX(value);
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<int>(optionIndex, valueIndex, fixedValue));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
    else if (saneType == SANE_TYPE_INT)
    {
        // FIXME: drop down item should have the real value so we don't need to parse it
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        auto value = std::stoi(itemName);
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<int>(optionIndex, valueIndex, value));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
    else if (saneType == SANE_TYPE_STRING)
    {
        auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(selectedItem));
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<std::string>(optionIndex, valueIndex, itemName));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
}

void ZooScan::DeviceOptionsPanel::OnNumericTextFieldChanged(GtkEventControllerFocus *focusController)
{
    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(focusController));
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto option = m_DeviceOptions->GetOption(optionIndex);
    auto saneType = option->GetValueType();

    if ((saneType == SANE_TYPE_FIXED) || (saneType == SANE_TYPE_INT))
    {
        auto *textField = GTK_ENTRY(widget);
        auto *entryBuffer = gtk_entry_get_buffer(textField);
        auto text = gtk_entry_buffer_get_text(entryBuffer);
        int value;

        try
        {
            if (saneType == SANE_TYPE_FIXED)
            {
                double v = std::stod(text);
                value = SANE_FIX(v);
            }
            else
            {
                value = std::stoi(text);
            }
        }
        catch (std::invalid_argument)
        {
            value = 0;
        }
        catch (std::out_of_range)
        {
            value = 0;
        }

        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<int>(optionIndex, valueIndex, value));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
}

void ZooScan::DeviceOptionsPanel::OnStringTextFieldChanged(GtkEventControllerFocus *focusController)
{
    auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(focusController));
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto option = m_DeviceOptions->GetOption(optionIndex);
    auto saneType = option->GetValueType();

    if (saneType == SANE_TYPE_STRING)
    {
        auto *textField = GTK_ENTRY(widget);
        auto *entryBuffer = gtk_entry_get_buffer(textField);
        auto text = gtk_entry_buffer_get_text(entryBuffer);

        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<std::string>(optionIndex, valueIndex, text));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
}

void ZooScan::DeviceOptionsPanel::OnSpinButtonChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto option = m_DeviceOptions->GetOption(optionIndex);
    auto saneType = option->GetValueType();

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
            value = static_cast<int>(fieldValue);
        }

        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<int>(optionIndex, valueIndex, value));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
}

void ZooScan::DeviceOptionsPanel::Update(u_int64_t lastSeenVersion)
{
    auto firstChangesetVersion = m_DeviceOptions->FirstChangesetVersion();
    auto changeset = m_DeviceOptions->GetAggregatedChangeset(lastSeenVersion);

    if (firstChangesetVersion > lastSeenVersion || changeset->RebuildAll())
    {
        BuildUI();
    }
    else
    {
        for (auto changedIndex: changeset->ChangedIndices())
        {
            auto option = m_DeviceOptions->GetOption(changedIndex.m_OptionIndex);
            auto settingValueType = option->GetValueType();
            auto widget = m_Widgets[changedIndex.Hash()];

            switch (settingValueType)
            {
                case SANE_TYPE_BOOL:
                {
                    auto boolOption = dynamic_cast<const DeviceOptionValue<bool> *>(option);
                    gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), boolOption->GetValue() != 0);
                    break;
                }

                case SANE_TYPE_INT:
                case SANE_TYPE_FIXED:
                {
                    auto intOption = dynamic_cast<const DeviceOptionValue<int> *>(option);
                    auto value = intOption->GetValue(changedIndex.m_ValueIndex);

                    if (GTK_IS_DROP_DOWN(widget))
                    {
                        auto valueStr = settingValueType == SANE_TYPE_FIXED ? std::to_string(SANE_UNFIX(value))
                                                                            : std::to_string(value);
                        auto items = gtk_drop_down_get_model(GTK_DROP_DOWN(widget));
                        uint32_t valuePosition = 0;
                        for (uint32_t i = 0; i < g_list_model_get_n_items(items); i++)
                        {
                            auto *item = G_OBJECT(g_list_model_get_item(items, i));
                            auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
                            if (strcmp(itemName, valueStr.c_str()) == 0)
                            {
                                valuePosition = i;
                                break;
                            }
                        }

                        gtk_drop_down_set_selected(GTK_DROP_DOWN(widget), valuePosition);
                    }
                    else if (GTK_IS_SPIN_BUTTON(widget))
                    {
                        double fieldValue = value;
                        if (settingValueType == SANE_TYPE_FIXED)
                        {
                            fieldValue = SANE_UNFIX(fieldValue);
                        }
                        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), fieldValue);
                    }
                    else if (GTK_IS_ENTRY(widget))
                    {
                        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
                        auto text = SaneIntOrFixedToString(value, option);
                        gtk_entry_buffer_set_text(entryBuffer, text.c_str(), static_cast<int>(text.length()));
                    }
                    break;
                }

                case SANE_TYPE_STRING:
                {
                    auto strOption = dynamic_cast<const DeviceOptionValue<std::string> *>(option);
                    auto value = strOption->GetValue();

                    if (GTK_IS_DROP_DOWN(widget))
                    {
                        auto items = gtk_drop_down_get_model(GTK_DROP_DOWN(widget));
                        uint32_t valuePosition = 0;
                        for (uint32_t i = 0; i < g_list_model_get_n_items(items); i++)
                        {
                            auto *item = G_OBJECT(g_list_model_get_item(items, i));
                            auto itemName = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
                            if (strcmp(itemName, value.c_str()) == 0)
                            {
                                valuePosition = i;
                                break;
                            }
                        }

                        gtk_drop_down_set_selected(GTK_DROP_DOWN(widget), valuePosition);
                    }
                    else if (GTK_IS_ENTRY(widget))
                    {
                        auto *entryBuffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
                        gtk_entry_buffer_set_text(entryBuffer, value.c_str(), static_cast<int>(value.length()));
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}
