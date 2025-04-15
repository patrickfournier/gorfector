#include "DeviceOptionsPanel.hpp"
#include <memory>
#include <utility>
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

void AddWidgetToParent(GtkWidget *parent, GtkWidget *child)
{
    if (ADW_IS_PREFERENCES_GROUP(parent))
    {
        adw_preferences_group_add(ADW_PREFERENCES_GROUP(parent), child);
    }
    else if (ADW_IS_EXPANDER_ROW(parent))
    {
        adw_expander_row_add_row(ADW_EXPANDER_ROW(parent), child);
    }
    else if (ADW_IS_PREFERENCES_PAGE(parent) && ADW_IS_PREFERENCES_GROUP(child))
    {
        adw_preferences_page_add(ADW_PREFERENCES_PAGE(parent), ADW_PREFERENCES_GROUP(child));
    }
    else
    {
        gtk_box_append(GTK_BOX(parent), child);
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
        GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex)
{
    auto boolOption = dynamic_cast<const DeviceOptionValue<bool> *>(option);
    if (boolOption == nullptr)
        return;

    auto checkButton = adw_switch_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(checkButton), option->GetTitle());
    if (option->GetDescription() != nullptr && strlen(option->GetDescription()) > 0)
    {
        adw_action_row_set_subtitle(ADW_ACTION_ROW(checkButton), option->GetDescription());
    }
    adw_switch_row_set_active(ADW_SWITCH_ROW(checkButton), boolOption->GetValue() != 0);

    if (option->IsDisplayOnly())
    {
        gtk_widget_set_sensitive(checkButton, false);
    }
    else
    {
        gtk_widget_set_sensitive(checkButton, true);
        // ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget,
        // "notify::selected");
    }

    AddWidgetToParent(parent, checkButton);

    auto index = WidgetIndex(settingIndex, 0);
    g_object_set_data(G_OBJECT(checkButton), "OptionIndex", GINT_TO_POINTER(index.Hash()));
    m_Widgets[index.Hash()] = checkButton;

    // ConnectGtkSignalWithParamSpecs(this, &DeviceSelector::OnActivateNetwork, checkButton, "notify::active");
    // ConnectGtkSignal(this, &DeviceOptionsPanel::OnCheckBoxChanged, checkButton, "toggled");
}

void ZooScan::DeviceOptionsPanel::AddVectorRow(
        GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex,
        bool multiValue = false)
{
    auto intOption = dynamic_cast<const DeviceOptionValue<int> *>(option);
    if (intOption == nullptr)
        return;

    std::string title(multiValue ? std::to_string(valueIndex) : option->GetTitle());
    std::string description(multiValue ? "" : option->GetDescription());
    int value = intOption->GetValue(valueIndex);

    if (!multiValue)
    {
        // Add unit label
        if (auto unitStr = SaneUnitToString(option->GetUnit()); unitStr != nullptr && strlen(unitStr) > 0)
        {
            title += " (" + std::string(unitStr) + ")";
        }
    }

    GtkWidget *valueWidget = nullptr;
    if (auto range = option->GetRange(); range != nullptr && range->quant != 0)
    {
        double min, max, step, fieldValue;
        if (option->GetValueType() == SANE_TYPE_FIXED)
        {
            min = SANE_UNFIX(range->min);
            max = SANE_UNFIX(range->max);
            step = SANE_UNFIX(range->quant);
            fieldValue = SANE_UNFIX(value);
        }
        else
        {
            min = range->min;
            max = range->max;
            step = range->quant;
            fieldValue = value;
        }

        valueWidget = adw_spin_row_new_with_range(min, max, step);
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title.c_str());
        adw_action_row_set_subtitle(ADW_ACTION_ROW(valueWidget), description.c_str());
        adw_spin_row_set_value(ADW_SPIN_ROW(valueWidget), fieldValue);

        // ConnectGtkSignal(this, &DeviceOptionsPanel::OnSpinButtonChanged, valueWidget, "value-changed");
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

        valueWidget = adw_combo_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title.c_str());
        adw_action_row_set_subtitle(ADW_ACTION_ROW(valueWidget), description.c_str());
        auto *stringList = gtk_string_list_new(options.get());
        adw_combo_row_set_model(ADW_COMBO_ROW(valueWidget), G_LIST_MODEL(stringList));
        adw_combo_row_set_selected(ADW_COMBO_ROW(valueWidget), activeIndex);

        // ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget,
        // "notify::selected");
    }
    else // no constraint
    {
        auto valueStr = SaneIntOrFixedToString(value, option);
        valueWidget = adw_entry_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title.c_str());
        gtk_widget_set_tooltip_text(valueWidget, description.c_str());
        gtk_editable_set_text(GTK_EDITABLE(valueWidget), valueStr.c_str());

        // ConnectGtkSignal(this, &DeviceOptionsPanel::OnNumericTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        if (option->IsDisplayOnly())
        {
            gtk_widget_set_sensitive(valueWidget, false);
        }
        else
        {
            gtk_widget_set_sensitive(valueWidget, true);
        }

        AddWidgetToParent(parent, valueWidget);

        auto index = WidgetIndex(settingIndex, valueIndex);
        g_object_set_data(G_OBJECT(valueWidget), "OptionIndex", GINT_TO_POINTER(index.Hash()));
        m_Widgets[index.Hash()] = valueWidget;
    }
}

void ZooScan::DeviceOptionsPanel::AddStringRow(
        GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex)
{
    auto strOption = dynamic_cast<const DeviceOptionValue<std::string> *>(option);
    if (strOption == nullptr)
        return;

    auto title = option->GetTitle();
    auto description = option->GetDescription();
    auto value = strOption->GetValue();

    GtkWidget *valueWidget = nullptr;
    if (auto stringList = option->GetStringList(); stringList != nullptr)
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

        valueWidget = adw_combo_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title);
        adw_action_row_set_subtitle(ADW_ACTION_ROW(valueWidget), description);
        auto *gStringList = gtk_string_list_new(stringList);
        adw_combo_row_set_model(ADW_COMBO_ROW(valueWidget), G_LIST_MODEL(gStringList));
        adw_combo_row_set_selected(ADW_COMBO_ROW(valueWidget), activeIndex);

        // ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget,
        // "notify::selected");
    }
    else // no constraint
    {
        valueWidget = adw_entry_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title);
        gtk_widget_set_tooltip_text(valueWidget, description);
        gtk_editable_set_text(GTK_EDITABLE(valueWidget), value.c_str());

        // ConnectGtkSignal(this, &DeviceOptionsPanel::OnNumericTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        if (option->IsDisplayOnly())
        {
            gtk_widget_set_sensitive(valueWidget, false);
        }
        else
        {
            gtk_widget_set_sensitive(valueWidget, true);
        }

        AddWidgetToParent(parent, valueWidget);

        auto index = WidgetIndex(settingIndex, 0);
        g_object_set_data(G_OBJECT(valueWidget), "OptionIndex", GINT_TO_POINTER(index.Hash()));
        m_Widgets[index.Hash()] = valueWidget;
    }
}

ZooScan::DeviceOptionsPanel::DeviceOptionsPanel(
        int saneInitId, std::string deviceName, ZooLib::CommandDispatcher *parentDispatcher, App *app)
    : m_App(app)
    , m_SaneInitId(saneInitId)
    , m_DeviceName(std::move(deviceName))
    , m_Dispatcher(parentDispatcher)
{
    if (m_DeviceName.empty())
        return;

    m_DeviceOptions = new DeviceOptionsState(m_App->GetState(), m_DeviceName);

    m_OptionUpdateObserver = new ViewUpdateObserver(this, m_DeviceOptions);
    m_App->GetObserverManager()->AddObserver(m_OptionUpdateObserver);

    m_RootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    auto scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroller), FALSE);
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroller), FALSE);

    auto viewport = gtk_viewport_new(gtk_adjustment_new(0, 0, 0, 0, 0, 0), gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), viewport);

    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_viewport_set_child(GTK_VIEWPORT(viewport), box);

    auto viewSwitcher = adw_view_switcher_new();
    adw_view_switcher_set_policy(ADW_VIEW_SWITCHER(viewSwitcher), ADW_VIEW_SWITCHER_POLICY_WIDE);

    auto stack = adw_view_stack_new();
    adw_view_stack_set_hhomogeneous(ADW_VIEW_STACK(stack), TRUE);
    adw_view_stack_set_vhomogeneous(ADW_VIEW_STACK(stack), FALSE);
    adw_view_switcher_set_stack(ADW_VIEW_SWITCHER(viewSwitcher), ADW_VIEW_STACK(stack));
    gtk_widget_set_margin_start(stack, 10);
    gtk_widget_set_margin_end(stack, 10);
    gtk_box_append(GTK_BOX(box), stack);

    gtk_box_append(GTK_BOX(m_RootWidget), viewSwitcher);
    gtk_box_append(GTK_BOX(m_RootWidget), scroller);

    m_OptionParent = stack;
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

    if (m_PageBasic != nullptr)
    {
        adw_view_stack_remove(ADW_VIEW_STACK(m_OptionParent), m_PageBasic);
    }
    if (m_PageAdvanced != nullptr)
    {
        adw_view_stack_remove(ADW_VIEW_STACK(m_OptionParent), m_PageAdvanced);
    }

    m_PageBasic = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_view_stack_add_titled_with_icon(
            ADW_VIEW_STACK(m_OptionParent), m_PageBasic, "basic", "Basic", "settings-symbolic");

    m_PageAdvanced = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_view_stack_add_titled_with_icon(
            ADW_VIEW_STACK(m_OptionParent), m_PageAdvanced, "advanced", "Advanced",
            "settings-symbolic"); // FIXME use wrench-wide-symbolic

    AddCommonOptions();
    AddOtherOptions();
}

void ZooScan::DeviceOptionsPanel::AddCommonOptions()
{
    auto optionGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(optionGroup), "Common Options");
    gtk_widget_set_margin_bottom(optionGroup, 10);
    gtk_widget_set_margin_top(optionGroup, 10);
    AddWidgetToParent(m_PageBasic, optionGroup);

    if (m_DeviceOptions->XResolutionIndex() != DeviceOptionsState::k_InvalidIndex)
    {
        AddOptionRow(m_DeviceOptions->XResolutionIndex(), m_PageBasic, optionGroup, false, false);
        AddOptionRow(m_DeviceOptions->YResolutionIndex(), m_PageBasic, optionGroup, false, false);
    }
    else
    {
        AddOptionRow(m_DeviceOptions->ResolutionIndex(), m_PageBasic, optionGroup, false, false);
    }

    auto *expander = adw_expander_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(expander), "Scan Area");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(optionGroup), expander);

    AddOptionRow(m_DeviceOptions->TLXIndex(), m_PageBasic, expander, false, false);
    AddOptionRow(m_DeviceOptions->TLYIndex(), m_PageBasic, expander, false, false);
    AddOptionRow(m_DeviceOptions->BRXIndex(), m_PageBasic, expander, false, false);
    AddOptionRow(m_DeviceOptions->BRYIndex(), m_PageBasic, expander, false, false);

    if (m_DeviceOptions->ModeIndex() != DeviceOptionsState::k_InvalidIndex)
    {
        AddOptionRow(m_DeviceOptions->ModeIndex(), m_PageBasic, optionGroup, false, false);
    }

    if (m_DeviceOptions->BitDepthIndex() != DeviceOptionsState::k_InvalidIndex)
    {
        AddOptionRow(m_DeviceOptions->BitDepthIndex(), m_PageBasic, optionGroup, false, false);
    }
}

void ZooScan::DeviceOptionsPanel::AddOtherOptions()
{
    auto parent = m_PageBasic;
    for (auto optionIndex = 0UL; optionIndex < m_DeviceOptions->GetOptionCount(); optionIndex++)
    {
        parent = AddOptionRow(optionIndex, m_PageBasic, parent, false, true);
    }

    parent = m_PageAdvanced;
    for (auto optionIndex = 0UL; optionIndex < m_DeviceOptions->GetOptionCount(); optionIndex++)
    {
        parent = AddOptionRow(optionIndex, m_PageAdvanced, parent, true, false);
    }
}

GtkWidget *ZooScan::DeviceOptionsPanel::AddOptionRow(
        uint64_t optionIndex, GtkWidget *page, GtkWidget *parent, bool skipBasicOptions, bool skipAdvancedOptions)
{
    auto device = m_App->GetDeviceByName(m_DeviceName);
    auto optionDescriptor = device->GetOptionDescriptor(optionIndex);
    const auto *optionValue = m_DeviceOptions->GetOption(optionIndex);
    if (optionDescriptor == nullptr)
    {
        return parent;
    }

    if (DeviceOptionValueBase::ShouldHide(*optionDescriptor))
    {
        return parent;
    }

    if ((optionDescriptor->type != SANE_TYPE_GROUP) &&
        ((DeviceOptionValueBase::IsAdvanced(*optionDescriptor) && skipAdvancedOptions) ||
         (!DeviceOptionValueBase::IsAdvanced(*optionDescriptor) && skipBasicOptions)))
    {
        return parent;
    }

    // This is used to avoid creating empty groups: only create the pending group if there is at least
    // one option in it.
    static GtkWidget *pendingGroup = nullptr;
    if (optionDescriptor->type != SANE_TYPE_GROUP && pendingGroup != nullptr)
    {
        AddWidgetToParent(page, pendingGroup);

        pendingGroup = nullptr;
    }

    switch (optionDescriptor->type)
    {
        case SANE_TYPE_BOOL:
        {
            if (optionValue != nullptr)
            {
                AddCheckButton(parent, optionValue, optionIndex);
            }
            break;
        }

        case SANE_TYPE_INT:
        case SANE_TYPE_FIXED:
        {
            if (optionValue != nullptr)
            {
                auto numberOfElements = optionValue->GetValueCount();
                if (numberOfElements > 4)
                    break;

                GtkWidget *vectorBox;
                if (numberOfElements == 1)
                {
                    vectorBox = parent;
                }
                else
                {
                    vectorBox = adw_expander_row_new();
                    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(vectorBox), optionValue->GetTitle());
                    if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
                    {
                        adw_expander_row_set_subtitle(ADW_EXPANDER_ROW(vectorBox), optionDescriptor->desc);
                    }

                    AddWidgetToParent(parent, vectorBox);
                }

                for (auto i = 0U; i < numberOfElements; i++)
                {
                    AddVectorRow(vectorBox, optionValue, optionIndex, i, numberOfElements > 1);
                }
            }
            break;
        }

        case SANE_TYPE_STRING:
        {
            if (optionValue != nullptr)
            {
                AddStringRow(parent, optionValue, optionIndex);
            }
            break;
        }

        case SANE_TYPE_GROUP:
        {
            auto group = adw_preferences_group_new();
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(group), optionDescriptor->title);
            gtk_widget_set_margin_bottom(group, 10);
            gtk_widget_set_margin_top(group, 10);
            if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
            {
                adw_preferences_group_set_description(ADW_PREFERENCES_GROUP(group), optionDescriptor->desc);
            }

            pendingGroup = group;
            parent = group;
            break;
        }

        case SANE_TYPE_BUTTON:
        {
            auto *button = gtk_button_new_with_label(optionDescriptor->title);
            if (optionDescriptor->desc != nullptr && strlen(optionDescriptor->desc) > 0)
            {
                gtk_widget_set_tooltip_text(button, optionDescriptor->desc);
            }

            if (optionValue->IsDisplayOnly())
            {
                gtk_widget_set_sensitive(button, false);
            }
            else
            {
                gtk_widget_set_sensitive(button, true);
            }

            AddWidgetToParent(parent, button);

            break;
        }

        default:
            break;
    }

    return parent;
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
        catch (std::invalid_argument &)
        {
            value = 0;
        }
        catch (std::out_of_range &)
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

void ZooScan::DeviceOptionsPanel::Update(const std::vector<uint64_t> &lastSeenVersions)
{
    auto firstChangesetVersion = m_DeviceOptions->FirstChangesetVersion();
    auto changeset = m_DeviceOptions->GetAggregatedChangeset(lastSeenVersions[0]);

    if (firstChangesetVersion > lastSeenVersions[0] || changeset->RebuildAll())
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
