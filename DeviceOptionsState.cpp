#include <cstring>
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

void ZooScan::DeviceOptionsState::BuildOptions()
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

void ZooScan::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, bool requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<bool> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
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
        BuildOptions();
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
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

void ZooScan::DeviceOptionsState::Updater::SetOptionValue(
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

void ZooScan::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, int requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    // TODO: apply SANE constraints

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
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
        BuildOptions();
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
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

void ZooScan::DeviceOptionsState::Updater::SetOptionValue(
        uint32_t optionIndex, uint32_t valueIndex, const std::string &requestedValue) const
{
    auto *option = dynamic_cast<DeviceOptionValue<std::string> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(
                std::string("Failed to set option (type mismatch): ") +
                m_StateComponent->m_OptionValues[optionIndex]->GetTitle());
    }

    // TODO: apply SANE constraints

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
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
        BuildOptions();
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
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

void ZooScan::DeviceOptionsState::Updater::DeserializeAndApply(const nlohmann::json &json) const
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

    if (json.contains("device"))
    {
        auto device = json["device"];
        if (device.contains("name"))
        {
            if (strcmp(saneDevice->Name(), device["name"].get<std::string>().c_str()) != 0)
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

    std::vector<size_t> changedIndices;
    for (auto optionIndex = 0UL; optionIndex < m_StateComponent->m_OptionValues.size(); optionIndex++)
    {
        auto optionValue = m_StateComponent->m_OptionValues[optionIndex];
        if (optionValue == nullptr)
        {
            continue;
        }

        if (json.contains("options"))
        {
            auto options = json["options"];
            if (options.contains(optionValue->GetName()))
            {
                if (optionValue->Deserialize(options[optionValue->GetName()]))
                {
                    changedIndices.push_back(optionIndex);
                    for (auto valueIndex = 0U; valueIndex < optionValue->GetValueCount(); valueIndex++)
                    {
                        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
                    }
                }
            }
        }
    }

    // Initial set of options
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
                option->SetDeviceValue(0, std::string(saneValue.get()));
                break;
            }

            default:
                break;
        }
    }

    // Check that the requested values were not changed by another option. Reapply if necessary.
    bool repeat = true;
    int maxRepeatCount = 10;
    while (repeat && maxRepeatCount > 0)
    {
        repeat = false;
        maxRepeatCount--;

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

                    SANE_Bool expectedValue = option->GetValue(0) ? SANE_TRUE : SANE_FALSE;
                    SANE_Bool currentValue;
                    saneDevice->GetOptionValue(changedIndex, &currentValue);
                    if (currentValue != expectedValue)
                    {
                        SANE_Bool saneValue = option->GetRequestedValue(0) ? SANE_TRUE : SANE_FALSE;
                        saneDevice->SetOptionValue(changedIndex, &saneValue, &optionInfo);
                        option->SetDeviceValue(0, saneValue == SANE_TRUE);
                        repeat = true;
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
                                std::string("Failed to set option (type mismatch): ") + optionValue->GetTitle());
                    }

                    auto valueCount = option->GetValueCount();
                    std::unique_ptr<SANE_Word[]> currentValue(new SANE_Word[valueCount]);
                    saneDevice->GetOptionValue(changedIndex, currentValue.get());
                    bool asExpected = true;
                    for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                    {
                        if (currentValue[valueIndex] != option->GetValue(valueIndex))
                        {
                            asExpected = false;
                            break;
                        }
                    }

                    if (!asExpected)
                    {
                        std::unique_ptr<SANE_Word[]> saneValue(new SANE_Word[valueCount]);
                        for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                        {
                            saneValue[valueIndex] = option->GetRequestedValue(valueIndex);
                        }

                        saneDevice->SetOptionValue(changedIndex, saneValue.get(), &optionInfo);

                        for (auto valueIndex = 0U; valueIndex < valueCount; valueIndex++)
                        {
                            option->SetDeviceValue(valueIndex, saneValue[valueIndex]);
                        }
                        repeat = true;
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

                    std::string expectedValue = option->GetValue(0);
                    std::unique_ptr<char[]> currentValue(new char[option->GetValueSize() + 1]);
                    saneDevice->GetOptionValue(changedIndex, currentValue.get());

                    if (strcmp(expectedValue.c_str(), currentValue.get()) != 0)
                    {
                        std::unique_ptr<char[]> saneValue(new char[option->GetValueSize() + 1]);
                        strcpy(saneValue.get(), option->GetRequestedValue(0).c_str());
                        saneDevice->SetOptionValue(changedIndex, saneValue.get(), &optionInfo);
                        option->SetDeviceValue(0, std::string(saneValue.get()));
                        repeat = true;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

bool ZooScan::DeviceOptionsState::IsPreview() const
{
    auto optionValue = m_OptionValues[m_PreviewIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "preview") == 0))
    {
        return dynamic_cast<DeviceOptionValue<bool> *>(optionValue)->GetValue(0) != 0;
    }

    return false;
}

std::string ZooScan::DeviceOptionsState::GetMode() const
{
    auto optionValue = m_OptionValues[m_ModeIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "mode") == 0))
    {
        return dynamic_cast<DeviceOptionValue<std::string> *>(optionValue)->GetValue(0);
    }

    return "";
}

ZooScan::ScanAreaUnit ZooScan::DeviceOptionsState::GetScanAreaUnit() const
{
    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "tl-x") == 0))
    {
        return optionValue->GetUnit() == SANE_UNIT_DPI ? ScanAreaUnit::Pixels : ScanAreaUnit::Millimeters;
    }

    return ScanAreaUnit::Pixels;
}

ZooScan::Rect<double> ZooScan::DeviceOptionsState::GetScanArea() const
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

ZooScan::Rect<double> ZooScan::DeviceOptionsState::GetMaxScanArea() const
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

int ZooScan::DeviceOptionsState::GetResolution() const
{
    auto optionValue = m_OptionValues[m_ResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return 0;
}

int ZooScan::DeviceOptionsState::GetXResolution() const
{
    auto optionValue = m_OptionValues[m_XResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "x-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int ZooScan::DeviceOptionsState::GetYResolution() const
{
    auto optionValue = m_OptionValues[m_YResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "y-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int ZooScan::DeviceOptionsState::GetBitDepth() const
{
    auto optionValue = m_OptionValues[m_BitDepthIndex];
    if (optionValue != nullptr && (strcmp(optionValue->GetName(), "depth") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int> *>(optionValue)->GetValue(0);
    }

    return 8;
}

nlohmann::json *ZooScan::DeviceOptionsState::Serialize() const
{
    auto device = GetDevice();
    if (device == nullptr)
    {
        return nullptr;
    }

    auto json = new nlohmann::json();

    (*json)["device"]["name"] = device->Name();
    (*json)["device"]["vendor"] = device->Vendor();
    (*json)["device"]["model"] = device->Model();
    (*json)["device"]["type"] = device->Type();

    (*json)["options"] = nlohmann::json::object();
    for (auto optionValue: m_OptionValues)
    {
        if (optionValue == nullptr)
        {
            continue;
        }
        if (!optionValue->IsSoftwareSettable())
        {
            continue;
        }

        optionValue->Serialize((*json)["options"]);
    }

    return json;
}
