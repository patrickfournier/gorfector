#include <cstring>
#include <glib.h>
#include <memory>

#include "DeviceOptionsState.hpp"
#include "SaneDevice.hpp"
#include "SaneException.hpp"

static int EnsureBufferSize(std::unique_ptr<char8_t[]> &buffer, int currentSize, int requestedSize)
{
    if (requestedSize > (1 << 24))
    {
        return currentSize;
    }

    if (currentSize < requestedSize)
    {
        buffer.reset(new char8_t[requestedSize]);
        return requestedSize;
    }

    return currentSize;
}

void Gorfector::DeviceOptionsState::ReloadOptions() const
{
    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    int optionCount;
    device->GetOptionValue(0, &optionCount);

    int valueBufferSize = sizeof(SANE_Word) * 512;
    std::unique_ptr<char8_t[]> value(new char8_t[valueBufferSize]);

    for (auto optionIndex = 1; optionIndex < optionCount; optionIndex++)
    {
        const SANE_Option_Descriptor *optionDescriptor = device->GetOptionDescriptor(optionIndex);
        if (optionDescriptor == nullptr)
        {
            continue;
        }

        if (optionDescriptor->type != SANE_TYPE_GROUP && optionDescriptor->type != SANE_TYPE_BUTTON)
        {
            valueBufferSize = EnsureBufferSize(value, valueBufferSize, optionDescriptor->size);
            if (valueBufferSize < optionDescriptor->size)
            {
                throw SaneException(
                        std::string(optionDescriptor->name) + ": Failed to allocate buffer for option value.");
            }

            device->GetOptionValue(optionIndex, value.get());
        }

        switch (optionDescriptor->type)
        {
            case SANE_TYPE_BOOL:
            {
                auto *settingValue = dynamic_cast<DeviceOptionValue<bool> *>(m_OptionValues[optionIndex]);
                if (settingValue != nullptr)
                {
                    settingValue->SetDeviceValue(0, (*reinterpret_cast<SANE_Bool *>(value.get())));
                }
                else
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") +
                            m_OptionValues[optionIndex]->GetTitle());
                }
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                int numberOfElements = static_cast<int>(optionDescriptor->size / sizeof(SANE_Word));
                if (numberOfElements > 4)
                    break;

                auto *intValue = dynamic_cast<DeviceOptionValue<int> *>(m_OptionValues[optionIndex]);
                if (intValue != nullptr)
                {
                    for (auto i = 0; i < numberOfElements; i++)
                    {
                        intValue->SetDeviceValue(i, reinterpret_cast<SANE_Int *>(value.get())[i]);
                    }
                }
                else
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") +
                            m_OptionValues[optionIndex]->GetTitle());
                }
                break;
            }

            case SANE_TYPE_STRING:
            {
                auto *strValue = dynamic_cast<DeviceOptionValue<std::string> *>(m_OptionValues[optionIndex]);
                if (strValue != nullptr)
                {
                    strValue->SetDeviceValue(0, std::string(reinterpret_cast<SANE_String>(value.get())));
                }
                else
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") +
                            m_OptionValues[optionIndex]->GetTitle());
                }
                break;
            }

            default:
                break;
        }
    }
}

void Gorfector::DeviceOptionsState::BuildOptions()
{
    Clear();

    auto device = GetDevice();
    if (device == nullptr)
    {
        return;
    }

    int optionCount;
    device->GetOptionValue(0, &optionCount);

    int valueBufferSize = sizeof(SANE_Word) * 512;
    std::unique_ptr<char8_t[]> value(new char8_t[valueBufferSize]);

    for (auto optionIndex = 1; optionIndex < optionCount; optionIndex++)
    {
        const SANE_Option_Descriptor *optionDescriptor = device->GetOptionDescriptor(optionIndex);
        if (optionDescriptor == nullptr)
        {
            continue;
        }

        if (optionDescriptor->type != SANE_TYPE_GROUP && optionDescriptor->type != SANE_TYPE_BUTTON)
        {
            valueBufferSize = EnsureBufferSize(value, valueBufferSize, optionDescriptor->size);
            if (valueBufferSize < optionDescriptor->size)
            {
                throw SaneException(
                        std::string(optionDescriptor->name) + ": Failed to allocate buffer for option value.");
            }

            device->GetOptionValue(optionIndex, value.get());
        }

        switch (optionDescriptor->type)
        {
            case SANE_TYPE_BOOL:
            {
                auto *settingValue = new DeviceOptionValue<bool>(optionDescriptor);
                settingValue->SetValues(
                        0, (*reinterpret_cast<SANE_Bool *>(value.get())),
                        (*reinterpret_cast<SANE_Bool *>(value.get())));
                AddOptionValue(optionIndex, settingValue);
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                int numberOfElements = static_cast<int>(optionDescriptor->size / sizeof(SANE_Word));
                if (numberOfElements > 4)
                    break;

                auto *intValue = new DeviceOptionValue<int>(optionDescriptor);
                for (auto i = 0; i < numberOfElements; i++)
                {
                    intValue->SetValues(
                            i, reinterpret_cast<SANE_Int *>(value.get())[i],
                            reinterpret_cast<SANE_Int *>(value.get())[i]);
                }
                AddOptionValue(optionIndex, intValue);
                break;
            }

            case SANE_TYPE_STRING:
            {
                auto *strValue = new DeviceOptionValue<std::string>(optionDescriptor);
                strValue->SetValues(
                        0, std::string(reinterpret_cast<SANE_String>(value.get())),
                        std::string(reinterpret_cast<SANE_String>(value.get())));
                AddOptionValue(optionIndex, strValue);
                break;
            }

            default:
                break;
        }
    }
}

void Gorfector::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, bool requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<bool> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    if (option->GetValue(valueIndex) == requestedValue)
    {
        option->SetRequestedValue(valueIndex, requestedValue);
        return;
    }

    int optionInfo;
    // Convert value to SANE type.
    SANE_Bool saneValue = requestedValue ? SANE_TRUE : SANE_FALSE;
    // Set the value on the device.
    m_StateComponent->GetDevice()->SetOptionValue(optionIndex, &saneValue, &optionInfo);

    // Check if the change of value on the device requires reloading all options.
    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        ReloadOptions();
        option = dynamic_cast<DeviceOptionValue<bool> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(
                    std::string("Failed to set option (type mismatch): ") +
                    m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
        }
        // Save the value as the requested value (which may differ from the actual value).
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        // No option reload required; the device accepted the value as-is. Update the requested value and the actual
        // value.
        option->SetValues(valueIndex, requestedValue, saneValue);
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(
                WidgetIndex{.OptionValueIndices = {optionIndex, valueIndex}});
    }
}

void Gorfector::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, double requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    SANE_Word requestedValueAsInt;
    if (option->GetValueType() == SANE_TYPE_FIXED)
    {
        requestedValueAsInt = SANE_FIX(requestedValue);
    }
    else
    {
        requestedValueAsInt = static_cast<SANE_Word>(requestedValue);
    }

    SetOptionValue(optionIndex, valueIndex, requestedValueAsInt);
}

void Gorfector::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, int requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    if (option->GetValue(valueIndex) == requestedValue)
    {
        option->SetRequestedValue(valueIndex, requestedValue);
        return;
    }

    auto device = m_StateComponent->GetDevice();
    if (device == nullptr)
    {
        return;
    }

    auto valueCount = option->GetValueCount();
    int optionInfo;
    std::unique_ptr<SANE_Word[]> saneValue(new SANE_Word[valueCount]);
    if (valueCount > 1)
    {
        device->GetOptionValue(optionIndex, saneValue.get());
        saneValue[valueIndex] = requestedValue;
    }
    else
    {
        saneValue[0] = requestedValue;
    }

    // Set the value on the device.
    device->SetOptionValue(optionIndex, saneValue.get(), &optionInfo);

    // Check if the change of value on the device requires reloading all options.
    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        ReloadOptions();
        option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(
                    std::string("Failed to set option (type mismatch): ") +
                    m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
        }
        // Save the value as the requested value (which may differ from the actual value).
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        // No option reload required; the device accepted the value as-is. Update the requested value and the actual
        // value.
        option->SetValues(valueIndex, requestedValue, saneValue[valueIndex]);
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(
                WidgetIndex{.OptionValueIndices = {optionIndex, valueIndex}});
    }
}

void Gorfector::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, const std::string &requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<std::string> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    if (option->GetValue(valueIndex) == requestedValue)
    {
        option->SetRequestedValue(valueIndex, requestedValue);
        return;
    }

    auto device = m_StateComponent->GetDevice();
    if (device == nullptr)
    {
        return;
    }

    int optionInfo;
    std::unique_ptr<char[]> saneValue(new char[option->GetValueSize() + 1]);
    strcpy(saneValue.get(), requestedValue.c_str());
    // Set the value on the device.
    device->SetOptionValue(optionIndex, saneValue.get(), &optionInfo);

    // Check if the change of value on the device requires reloading all options.
    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        ReloadOptions();
        option = dynamic_cast<DeviceOptionValue<std::string> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(
                    std::string("Failed to set option (type mismatch): ") +
                    m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
        }
        // Save the value as the requested value (which may differ from the actual value).
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        // No option reload required; the device accepted the value as-is. Update the requested value and the actual
        // value.
        option->SetValues(valueIndex, requestedValue, std::string(saneValue.get()));
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(
                WidgetIndex{.OptionValueIndices = {optionIndex, valueIndex}});
    }
}

void Gorfector::DeviceOptionsState::Updater::ApplyRequestedValuesToDevice(const std::vector<size_t> &changedIndices)
{
    auto saneDevice = m_StateComponent->GetDevice();
    if (saneDevice == nullptr)
    {
        return;
    }

    bool needReload = false;

    for (unsigned long changedIndex: changedIndices)
    {
        int optionInfo;
        auto optionValue = m_StateComponent->m_OptionValues[changedIndex];
        if (optionValue == nullptr)
        {
            continue;
        }

        switch (optionValue->GetValueType())
        {
            case SANE_TYPE_BOOL:
            {
                auto *option = dynamic_cast<DeviceOptionValue<bool> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") + optionValue->GetTitle());
                }

                SANE_Bool saneValue = option->GetRequestedValue(0) ? SANE_TRUE : SANE_FALSE;
                saneDevice->SetOptionValue(changedIndex, &saneValue, &optionInfo);
                needReload = needReload || (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;
                option->SetDeviceValue(0, saneValue == SANE_TRUE);
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                auto *option = dynamic_cast<DeviceOptionValue<int> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") + optionValue->GetTitle());
                }

                auto valueCount = option->GetValueCount();
                std::unique_ptr<SANE_Word[]> saneValue(new SANE_Word[valueCount]);
                for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                {
                    saneValue[valueIndex] = option->GetRequestedValue(valueIndex);
                }

                saneDevice->SetOptionValue(changedIndex, saneValue.get(), &optionInfo);
                needReload = needReload || (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;

                for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                {
                    option->SetDeviceValue(valueIndex, saneValue[valueIndex]);
                }
                break;
            }

            case SANE_TYPE_STRING:
            {
                auto *option = dynamic_cast<DeviceOptionValue<std::string> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to set option (type mismatch): ") + optionValue->GetTitle());
                }

                std::unique_ptr<char[]> saneValue(new char[option->GetValueSize() + 1]);
                strcpy(saneValue.get(), option->GetRequestedValue(0).c_str());
                saneDevice->SetOptionValue(changedIndex, saneValue.get(), &optionInfo);
                needReload = needReload || (optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0;
                option->SetDeviceValue(0, std::string(saneValue.get()));
                break;
            }

            default:
                break;
        }
    }

    if (needReload)
    {
        ReloadOptions();
    }
}

void Gorfector::DeviceOptionsState::Updater::FindRequestMismatches(
        const std::vector<size_t> &indicesToCheck, std::vector<size_t> &mismatches)
{
    for (unsigned long changedIndex: indicesToCheck)
    {
        auto optionValue = m_StateComponent->m_OptionValues[changedIndex];
        if (optionValue == nullptr)
        {
            continue;
        }

        switch (optionValue->GetValueType())
        {
            case SANE_TYPE_BOOL:
            {
                auto *option = dynamic_cast<DeviceOptionValue<bool> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to load option (type mismatch): ") + optionValue->GetTitle());
                }

                SANE_Bool expectedValue = option->GetRequestedValue(0) ? SANE_TRUE : SANE_FALSE;
                SANE_Bool currentValue = option->GetValue(0) ? SANE_TRUE : SANE_FALSE;
                if (currentValue != expectedValue)
                {
                    mismatches.push_back(changedIndex);
                }
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                auto *option = dynamic_cast<DeviceOptionValue<int> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to load option (type mismatch): ") + optionValue->GetTitle());
                }

                auto valueCount = option->GetValueCount();
                for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                {
                    SANE_Word expectedValue = option->GetRequestedValue(valueIndex);
                    SANE_Word currentValue = option->GetValue(valueIndex);

                    if (currentValue != expectedValue)
                    {
                        mismatches.push_back(changedIndex);
                        break;
                    }
                }
                break;
            }

            case SANE_TYPE_STRING:
            {
                auto *option = dynamic_cast<DeviceOptionValue<std::string> *>(optionValue);
                if (option == nullptr)
                {
                    throw std::runtime_error(
                            std::string("Failed to load option (type mismatch): ") + optionValue->GetTitle());
                }

                std::string expectedValue = option->GetRequestedValue(0);
                std::string currentValue = option->GetValue(0);
                if (currentValue != expectedValue)
                {
                    mismatches.push_back(changedIndex);
                }
                break;
            }

            default:
                break;
        }
    }
}

void Gorfector::DeviceOptionsState::Updater::ApplyPreset(const nlohmann::json &json)
{
    // Load the preset in m_OptionValues requested values.
    std::vector<size_t> indicesToApply;
    for (auto optionIndex = 0UL; optionIndex < m_StateComponent->m_OptionValues.size(); optionIndex++)
    {
        auto optionValue = m_StateComponent->m_OptionValues[optionIndex];
        if (optionValue == nullptr)
        {
            continue;
        }

        if (json.contains(optionValue->GetName()))
        {
            optionValue->Deserialize(json);
            indicesToApply.push_back(optionIndex);
            for (auto valueIndex = 0U; valueIndex < optionValue->GetValueCount(); valueIndex++)
            {
                m_StateComponent->GetCurrentChangeset()->AddChangedIndex(
                        WidgetIndex{.OptionValueIndices = {static_cast<uint32_t>(optionIndex), valueIndex}});
            }
        }
    }

    ApplyRequestedValuesToDevice(indicesToApply);

    // When applying the requested values to the device, the device may decide to change the value
    // of some other options (for example, setting the source to transparent may change the resolution).
    // In this case, we need to check if the requested values are still valid and reapply them if necessary.
    // This is done until all requested values are set or until we reach a maximum number of iterations.
    std::vector indiceSetA(indicesToApply);
    std::vector<size_t> indiceSetB(indicesToApply.size());

    auto currentIndiceSet = &indiceSetA;
    auto nextIndiceSet = &indiceSetB;
    int maxRepeatCount = 10;
    while (!currentIndiceSet->empty() && maxRepeatCount > 0)
    {
        maxRepeatCount--;

        nextIndiceSet->clear();
        FindRequestMismatches(*currentIndiceSet, *nextIndiceSet);
        ApplyRequestedValuesToDevice(*nextIndiceSet);

        std::swap(currentIndiceSet, nextIndiceSet);
    }

    if (!currentIndiceSet->empty())
    {
        g_warning("Failed to set all requested values on the device.");
    }
}

void Gorfector::DeviceOptionsState::Updater::LoadFromJson(const nlohmann::json &json)
{
    if (m_StateComponent->m_State == nullptr)
    {
        return;
    }

    auto saneDevice = m_StateComponent->GetDevice();
    if (saneDevice == nullptr)
    {
        return;
    }

    if (!json.contains(k_DeviceKey))
    {
        return;
    }

    auto device = json[k_DeviceKey];
    if (!device.contains(k_DeviceModelKey) || !device.contains(k_DeviceVendorKey))
    {
        return;
    }

    if (strcmp(saneDevice->GetVendor(), device[k_DeviceVendorKey].get<std::string>().c_str()) != 0)
    {
        return;
    }
    if (strcmp(saneDevice->GetModel(), device[k_DeviceModelKey].get<std::string>().c_str()) != 0)
    {
        return;
    }

    if (!json.contains(k_OptionsKey))
    {
        return;
    }

    ApplyPreset(json[k_OptionsKey]);
}

bool Gorfector::DeviceOptionsState::IsPreview() const
{
    auto optionValue = m_OptionValues[m_PreviewIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "preview") == 0))
    {
        return dynamic_cast<DeviceOptionValue<bool> *>(optionValue)->GetValue(0) != 0;
    }

    return false;
}

std::string Gorfector::DeviceOptionsState::GetMode() const
{
    auto optionValue = m_OptionValues[m_ModeIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "mode") == 0))
    {
        return dynamic_cast<DeviceOptionValue<std::string> *>(optionValue)->GetValue(0);
    }

    return "";
}

Gorfector::ScanAreaUnit Gorfector::DeviceOptionsState::GetScanAreaUnit() const
{
    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-x") == 0))
    {
        return optionValue->GetUnit() == SANE_UNIT_DPI ? ScanAreaUnit::e_Pixels : ScanAreaUnit::e_Millimeters;
    }

    return ScanAreaUnit::e_Pixels;
}

Gorfector::Rect<double> Gorfector::DeviceOptionsState::GetScanArea() const
{
    Rect<double> scanArea;
    SANE_Value_Type valueType;

    if (m_TLXIndex == std::numeric_limits<uint32_t>::max() || m_TLYIndex == std::numeric_limits<uint32_t>::max() ||
        m_BRXIndex == std::numeric_limits<uint32_t>::max() || m_BRYIndex == std::numeric_limits<uint32_t>::max())
    {
        return scanArea;
    }

    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-x") == 0))
    {
        const int left = dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
        valueType = optionValue->GetValueType();
        scanArea.x = valueType == SANE_TYPE_FIXED ? SANE_UNFIX(left) : static_cast<double>(left);
    }

    optionValue = m_OptionValues[m_TLYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-y") == 0))
    {
        const int top = dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
        valueType = optionValue->GetValueType();
        scanArea.y = valueType == SANE_TYPE_FIXED ? SANE_UNFIX(top) : static_cast<double>(top);
    }

    optionValue = m_OptionValues[m_BRXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "br-x") == 0))
    {
        const int right = dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
        valueType = optionValue->GetValueType();
        scanArea.width = valueType == SANE_TYPE_FIXED ? SANE_UNFIX(right) : static_cast<double>(right);
        scanArea.width -= scanArea.x;
    }

    optionValue = m_OptionValues[m_BRYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "br-y") == 0))
    {
        const int bottom = dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
        valueType = optionValue->GetValueType();
        scanArea.height = valueType == SANE_TYPE_FIXED ? SANE_UNFIX(bottom) : static_cast<double>(bottom);
        scanArea.height -= scanArea.y;
    }

    return scanArea;
}

Gorfector::Rect<double> Gorfector::DeviceOptionsState::GetMaxScanArea() const
{
    int left = 0, right = 0;
    int top = 0, bottom = 0;
    SANE_Value_Type valueType = SANE_TYPE_INT;

    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-x") == 0))
    {
        if (optionValue->IsRange())
        {
            left = optionValue->GetRange()->min;
            right = optionValue->GetRange()->max;
            valueType = optionValue->GetValueType();
        }
        else if (optionValue->IsNumberList())
        {
            auto list = optionValue->GetNumberList();
            left = std::numeric_limits<int>::max();
            right = std::numeric_limits<int>::min();
            for (auto i = 1; i < list[0]; i++)
            {
                left = std::min(left, list[i]);
                right = std::max(right, list[i]);
            }
        }
    }

    optionValue = m_OptionValues[m_TLYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-y") == 0))
    {
        if (optionValue->IsRange())
        {
            top = optionValue->GetRange()->min;
            bottom = optionValue->GetRange()->max;
        }
        else if (optionValue->IsNumberList())
        {
            auto list = optionValue->GetNumberList();
            top = std::numeric_limits<int>::max();
            bottom = std::numeric_limits<int>::min();
            for (auto i = 1; i < list[0]; i++)
            {
                top = std::min(top, list[i]);
                bottom = std::max(bottom, list[i]);
            }
        }
    }

    return Rect{
            valueType == SANE_TYPE_FIXED ? SANE_UNFIX(left) : left,
            valueType == SANE_TYPE_FIXED ? SANE_UNFIX(top) : top,
            valueType == SANE_TYPE_FIXED ? SANE_UNFIX(right - left) : right - left,
            valueType == SANE_TYPE_FIXED ? SANE_UNFIX(bottom - top) : bottom - top};
}

int Gorfector::DeviceOptionsState::GetResolution() const
{
    auto optionValue = m_OptionValues[m_ResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return 0;
}

int Gorfector::DeviceOptionsState::GetXResolution() const
{
    auto optionValue = m_OptionValues[m_XResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "x-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int Gorfector::DeviceOptionsState::GetYResolution() const
{
    auto optionValue = m_OptionValues[m_YResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "y-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int Gorfector::DeviceOptionsState::GetBitDepth() const
{
    auto optionValue = m_OptionValues[m_BitDepthIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "depth") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return 8;
}
