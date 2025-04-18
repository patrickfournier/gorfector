#include "DeviceOptionsPanel.hpp"

#include <memory>
#include <utility>

#include "Commands/ChangeOptionCommand.hpp"
#include "Gettext.hpp"
#include "OptionRewriter.hpp"
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
            return _("px");
        case SANE_UNIT_BIT:
            return _("bit");
        case SANE_UNIT_MM:
            return _("mm");
        case SANE_UNIT_DPI:
            return _("DPI");
        case SANE_UNIT_PERCENT:
            return _("%%");
        case SANE_UNIT_MICROSECOND:
            return _("μs");
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

void ZooScan::DeviceOptionsPanel::AddCheckButton(
        GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex)
{
    auto boolOption = dynamic_cast<const DeviceOptionValue<bool> *>(option);
    if (boolOption == nullptr)
        return;

    auto checkButton = adw_switch_row_new();
    auto title = m_Rewriter->GetTitle(settingIndex, option->GetTitle());
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(checkButton), title);
    auto description = m_Rewriter->GetDescription(settingIndex, option->GetDescription());
    if (description != nullptr && strlen(description) > 0)
    {
        adw_action_row_set_subtitle(ADW_ACTION_ROW(checkButton), description);
    }
    adw_switch_row_set_active(ADW_SWITCH_ROW(checkButton), boolOption->GetValue() != 0);

    auto isDisplayOnly = m_Rewriter->IsDisplayOnly(settingIndex, option->IsDisplayOnly());
    if (isDisplayOnly)
    {
        gtk_widget_set_sensitive(checkButton, false);
    }
    else
    {
        gtk_widget_set_sensitive(checkButton, true);
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnCheckBoxChanged, checkButton, "notify::active");
    }

    AddWidgetToParent(parent, checkButton);

    auto index = WidgetIndex(settingIndex, 0);
    g_object_set_data(G_OBJECT(checkButton), "OptionIndex", GINT_TO_POINTER(index.Hash()));
    m_Widgets[index.Hash()] = checkButton;
}

void ZooScan::DeviceOptionsPanel::AddVectorRow(
        GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex,
        bool multiValue = false)
{
    auto intOption = dynamic_cast<const DeviceOptionValue<int> *>(option);
    if (intOption == nullptr)
        return;

    std::string title(multiValue ? std::to_string(valueIndex) : m_Rewriter->GetTitle(settingIndex, option->GetTitle()));
    std::string description(multiValue ? "" : m_Rewriter->GetDescription(settingIndex, option->GetDescription()));
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
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnSpinButtonChanged, valueWidget, "notify::value");
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
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget, "notify::selected");
    }
    else // no constraint
    {
        auto valueStr = SaneIntOrFixedToString(value, option);
        valueWidget = adw_entry_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title.c_str());
        gtk_widget_set_tooltip_text(valueWidget, description.c_str());
        gtk_editable_set_text(GTK_EDITABLE(valueWidget), valueStr.c_str());

        auto focusController = gtk_event_controller_focus_new();
        gtk_widget_add_controller(valueWidget, focusController);
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnNumericTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        auto isDisplayOnly = m_Rewriter->IsDisplayOnly(settingIndex, option->IsDisplayOnly());
        if (isDisplayOnly)
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

    auto title(m_Rewriter->GetTitle(settingIndex, option->GetTitle()));
    auto description(m_Rewriter->GetDescription(settingIndex, option->GetDescription()));
    auto stringList = option->GetStringList();
    auto value = strOption->GetValue();

    GtkWidget *valueWidget = nullptr;
    if (stringList != nullptr)
    {
        auto stringListLen = 0;
        while (stringList[stringListLen] != nullptr)
        {
            stringListLen++;
        }
        stringListLen++; // for the nullptr element

        auto rewrittenStringList = new SANE_String_Const[stringListLen];
        m_Rewriter->GetStringList(settingIndex, stringList, rewrittenStringList);

        auto activeIndex = 0;
        auto index = 0;
        while (stringList[index] != nullptr)
        {
            auto optionValue = stringList[index];

            if (strcmp(optionValue, value.c_str()) == 0)
            {
                activeIndex = index;
                break;
            }
            index++;
        }

        valueWidget = adw_combo_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title);
        adw_action_row_set_subtitle(ADW_ACTION_ROW(valueWidget), description);
        auto *gStringList = gtk_string_list_new(rewrittenStringList);
        adw_combo_row_set_model(ADW_COMBO_ROW(valueWidget), G_LIST_MODEL(gStringList));
        adw_combo_row_set_selected(ADW_COMBO_ROW(valueWidget), activeIndex);
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnDropDownChanged, valueWidget, "notify::selected");

        delete[] rewrittenStringList;
    }
    else // no constraint
    {
        valueWidget = adw_entry_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(valueWidget), title);
        gtk_widget_set_tooltip_text(valueWidget, description);
        gtk_editable_set_text(GTK_EDITABLE(valueWidget), value.c_str());

        auto focusController = gtk_event_controller_focus_new();
        gtk_widget_add_controller(valueWidget, focusController);
        ConnectGtkSignalWithParamSpecs(this, &DeviceOptionsPanel::OnStringTextFieldChanged, focusController, "leave");
    }

    if (valueWidget != nullptr)
    {
        auto isDisplayOnly = m_Rewriter->IsDisplayOnly(settingIndex, option->IsDisplayOnly());
        if (isDisplayOnly)
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

    m_Rewriter = new OptionRewriter();
    auto device = m_App->GetDeviceByName(m_DeviceName);
    if (device != nullptr)
    {
        m_Rewriter->LoadOptionDescriptionFile(device->GetVendor(), device->GetModel());
    }

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
            ADW_VIEW_STACK(m_OptionParent), m_PageBasic, "basic", _("Basic"), "scanner-symbolic");

    m_PageAdvanced = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_view_stack_add_titled_with_icon(
            ADW_VIEW_STACK(m_OptionParent), m_PageAdvanced, "advanced", _("Advanced"), "scanner-symbolic");

    auto commonOptionIndices = AddCommonOptions();
    AddOtherOptions(commonOptionIndices);
}

std::vector<uint32_t> ZooScan::DeviceOptionsPanel::AddCommonOptions()
{
    auto optionGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(optionGroup), _("Common Options"));
    gtk_widget_set_margin_bottom(optionGroup, 10);
    gtk_widget_set_margin_top(optionGroup, 10);
    AddWidgetToParent(m_PageBasic, optionGroup);

    if (m_DeviceOptions->XResolutionIndex() != DeviceOptionsState::k_InvalidIndex)
    {
        AddOptionRow(m_DeviceOptions->XResolutionIndex(), optionGroup, nullptr, false, false);
        AddOptionRow(m_DeviceOptions->YResolutionIndex(), optionGroup, nullptr, false, false);
    }
    else
    {
        AddOptionRow(m_DeviceOptions->ResolutionIndex(), optionGroup, nullptr, false, false);
    }

    auto *expander = adw_expander_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(expander), _("Scan Area"));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(optionGroup), expander);

    AddOptionRow(m_DeviceOptions->TLXIndex(), expander, nullptr, false, false);
    AddOptionRow(m_DeviceOptions->TLYIndex(), expander, nullptr, false, false);
    AddOptionRow(m_DeviceOptions->BRXIndex(), expander, nullptr, false, false);
    AddOptionRow(m_DeviceOptions->BRYIndex(), expander, nullptr, false, false);

    return std::vector{m_DeviceOptions->ResolutionIndex(),  m_DeviceOptions->XResolutionIndex(),
                       m_DeviceOptions->YResolutionIndex(), m_DeviceOptions->TLXIndex(),
                       m_DeviceOptions->TLYIndex(),         m_DeviceOptions->BRXIndex(),
                       m_DeviceOptions->BRYIndex()};
}

void ZooScan::DeviceOptionsPanel::AddOtherOptions(const std::vector<uint32_t> &excludeIndices)
{
    auto parent = m_PageBasic;
    GtkWidget *pendingGroup = nullptr;
    for (auto optionIndex = 0UL; optionIndex < m_DeviceOptions->GetOptionCount(); optionIndex++)
    {
        if (std::ranges::find(excludeIndices, optionIndex) == excludeIndices.end())
        {
            std::tie(parent, pendingGroup) = AddOptionRow(optionIndex, parent, pendingGroup, false, true);
        }
    }

    parent = m_PageAdvanced;
    for (auto optionIndex = 0UL; optionIndex < m_DeviceOptions->GetOptionCount(); optionIndex++)
    {
        if (std::ranges::find(excludeIndices, optionIndex) == excludeIndices.end())
        {
            std::tie(parent, pendingGroup) = AddOptionRow(optionIndex, parent, pendingGroup, true, false);
        }
    }
}

std::tuple<GtkWidget *, GtkWidget *> ZooScan::DeviceOptionsPanel::AddOptionRow(
        uint64_t optionIndex, GtkWidget *parent, GtkWidget *pendingGroup, bool skipBasicOptions,
        bool skipAdvancedOptions)
{
    // We use the device to get all options, including those that have no associated value (like groups).
    auto device = m_App->GetDeviceByName(m_DeviceName);
    if (device == nullptr)
    {
        return {parent, pendingGroup};
    }

    auto optionDescriptor = device->GetOptionDescriptor(optionIndex);
    if (optionDescriptor == nullptr)
    {
        return {parent, pendingGroup};
    }

    // Do not display numeric values with more than 4 elements.
    auto isLargeVector = (optionDescriptor->type == SANE_TYPE_INT || optionDescriptor->type == SANE_TYPE_FIXED) &&
                         optionDescriptor->size > 4;
    auto shouldHide = SaneDevice::ShouldHide(*optionDescriptor) || isLargeVector;
    if (m_Rewriter->ShouldHide(optionIndex, m_Rewriter->ShouldHide(optionIndex, shouldHide)))
    {
        return {parent, pendingGroup};
    }

    auto isAdvanced = m_Rewriter->IsAdvanced(optionIndex, SaneDevice::IsAdvanced(*optionDescriptor));
    if ((optionDescriptor->type != SANE_TYPE_GROUP) &&
        ((isAdvanced && skipAdvancedOptions) || (!isAdvanced && skipBasicOptions)))
    {
        return {parent, pendingGroup};
    }

    // This is used to avoid creating empty groups: only create the pending group if there is at least
    // one option in it.
    if (optionDescriptor->type != SANE_TYPE_GROUP && pendingGroup != nullptr)
    {
        AddWidgetToParent(parent, pendingGroup);
        parent = pendingGroup;
        pendingGroup = nullptr;
    }

    // Get the value for the option.
    const auto *optionValue = m_DeviceOptions->GetOption(optionIndex);
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
                    auto title = m_Rewriter->GetTitle(optionIndex, optionDescriptor->title);
                    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(vectorBox), title);
                    auto description = m_Rewriter->GetDescription(optionIndex, optionDescriptor->desc);
                    if (description != nullptr && strlen(description) > 0)
                    {
                        adw_expander_row_set_subtitle(ADW_EXPANDER_ROW(vectorBox), description);
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
            auto title = m_Rewriter->GetTitle(optionIndex, optionDescriptor->title);
            adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(group), title);
            gtk_widget_set_margin_bottom(group, 10);
            gtk_widget_set_margin_top(group, 10);
            auto description = m_Rewriter->GetDescription(optionIndex, optionDescriptor->desc);
            if (description != nullptr && strlen(description) > 0)
            {
                adw_preferences_group_set_description(ADW_PREFERENCES_GROUP(group), description);
            }

            pendingGroup = group;
            break;
        }

        case SANE_TYPE_BUTTON:
        {
            auto title = m_Rewriter->GetTitle(optionIndex, optionDescriptor->title);
            auto *button = gtk_button_new_with_label(title);
            auto description = m_Rewriter->GetDescription(optionIndex, optionDescriptor->desc);
            if (description != nullptr && strlen(description) > 0)
            {
                gtk_widget_set_tooltip_text(button, description);
            }

            auto isDisplayOnly = m_Rewriter->IsDisplayOnly(optionIndex, optionValue->IsDisplayOnly());
            if (isDisplayOnly)
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

    return {parent, pendingGroup};
}

void ZooScan::DeviceOptionsPanel::OnCheckBoxChanged(GtkWidget *widget)
{
    auto gObjectData = g_object_get_data(G_OBJECT(widget), "OptionIndex");
    if (gObjectData == nullptr)
        return;

    auto index = WidgetIndex(reinterpret_cast<uint64_t>(gObjectData));
    auto optionIndex = index.m_OptionIndex;
    auto valueIndex = index.m_ValueIndex;

    auto *checkBox = ADW_SWITCH_ROW(widget);
    auto isChecked = adw_switch_row_get_active(checkBox);

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

    auto *dropDown = ADW_COMBO_ROW(widget);
    auto selectedItem = adw_combo_row_get_selected(dropDown);
    if (saneType == SANE_TYPE_FIXED)
    {
        auto valueList = option->GetNumberList();
        selectedItem = std::min(selectedItem + 1, static_cast<guint>(valueList[0]));
        auto value = valueList[selectedItem];
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand(optionIndex, valueIndex, value));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
    else if (saneType == SANE_TYPE_INT)
    {
        auto valueList = option->GetNumberList();
        selectedItem = std::min(selectedItem + 1, static_cast<guint>(valueList[0]));
        auto value = valueList[selectedItem];
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand(optionIndex, valueIndex, value));
        }
        catch (std::runtime_error &e)
        {
            ZooLib::ShowUserError(m_App->GetMainWindow(), e.what());
        }
    }
    else if (saneType == SANE_TYPE_STRING)
    {
        auto valueList = option->GetStringList();
        auto value = valueList[selectedItem];
        try
        {
            m_Dispatcher.Dispatch(ChangeOptionCommand<std::string>(optionIndex, valueIndex, value));
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
        auto *textField = GTK_EDITABLE(widget);
        auto text = gtk_editable_get_text(textField);
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
        auto *textField = GTK_EDITABLE(widget);
        auto text = gtk_editable_get_text(textField);

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
        auto *spinButton = ADW_SPIN_ROW(widget);
        auto fieldValue = adw_spin_row_get_value(spinButton);
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
                    adw_switch_row_set_active(ADW_SWITCH_ROW(widget), boolOption->GetValue() != 0);
                    break;
                }

                case SANE_TYPE_INT:
                case SANE_TYPE_FIXED:
                {
                    auto intOption = dynamic_cast<const DeviceOptionValue<int> *>(option);
                    auto value = intOption->GetValue(changedIndex.m_ValueIndex);

                    if (ADW_IS_COMBO_ROW(widget))
                    {
                        auto valueStr = settingValueType == SANE_TYPE_FIXED ? std::to_string(SANE_UNFIX(value))
                                                                            : std::to_string(value);
                        auto items = adw_combo_row_get_model(ADW_COMBO_ROW(widget));
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

                        adw_combo_row_set_selected(ADW_COMBO_ROW(widget), valuePosition);
                    }
                    else if (ADW_IS_SPIN_ROW(widget))
                    {
                        double fieldValue = value;
                        if (settingValueType == SANE_TYPE_FIXED)
                        {
                            fieldValue = SANE_UNFIX(fieldValue);
                        }
                        adw_spin_row_set_value(ADW_SPIN_ROW(widget), fieldValue);
                    }
                    else if (ADW_IS_ENTRY_ROW(widget))
                    {
                        auto text = SaneIntOrFixedToString(value, option);
                        gtk_editable_set_text(GTK_EDITABLE(widget), text.c_str());
                    }
                    break;
                }

                case SANE_TYPE_STRING:
                {
                    auto strOption = dynamic_cast<const DeviceOptionValue<std::string> *>(option);
                    auto value = strOption->GetValue();

                    if (ADW_IS_COMBO_ROW(widget))
                    {
                        auto items = adw_combo_row_get_model(ADW_COMBO_ROW(widget));
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

                        adw_combo_row_set_selected(ADW_COMBO_ROW(widget), valuePosition);
                    }
                    else if (ADW_IS_ENTRY_ROW(widget))
                    {
                        gtk_editable_set_text(GTK_EDITABLE(widget), value.c_str());
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}
