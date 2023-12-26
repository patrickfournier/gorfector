#include "DeviceOptionState.h"
#include "SaneException.h"
#include "SaneDevice.h"

#include <cstring>
#include <memory>

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

void ZooScan::DeviceOptionState::Updater::BuildOptions()
{
    Clear();

    int optionCount;
    m_StateComponent->m_Device->GetOptionValue(0, &optionCount);

    int valueBufferSize = sizeof(SANE_Word) * 512;
    std::unique_ptr<char8_t[]> value(new char8_t[valueBufferSize]);

    for (auto optionIndex = 1; optionIndex < optionCount; optionIndex++)
    {
        const SANE_Option_Descriptor *optionDescriptor = m_StateComponent->m_Device->GetOptionDescriptor(optionIndex);
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

            m_StateComponent->m_Device->GetOptionValue(optionIndex, value.get());
        }

        switch (optionDescriptor->type)
        {
            case SANE_TYPE_BOOL:
            {
                auto *settingValue = new DeviceOptionValue<bool>(optionDescriptor);
                settingValue->SetValue(0, (*reinterpret_cast<SANE_Bool *>(value.get())));
                AddOptionValue(optionIndex, settingValue);
                break;
            }

            case SANE_TYPE_INT:
            case SANE_TYPE_FIXED:
            {
                int numberOfElements = int(optionDescriptor->size / sizeof(SANE_Word));
                if (numberOfElements > 4)
                    break;

                auto *intValue = new DeviceOptionValue<int>(optionDescriptor);
                for (auto i = 0; i < numberOfElements; i++)
                {
                    intValue->SetValue(i, reinterpret_cast<SANE_Int *>(value.get())[i]);
                }
                AddOptionValue(optionIndex, intValue);
                break;
            }

            case SANE_TYPE_STRING:
            {
                auto *strValue = new DeviceOptionValue<std::string>(optionDescriptor);
                strValue->SetValue(0, std::string(reinterpret_cast<SANE_String>(value.get())));
                AddOptionValue(optionIndex, strValue);
                break;
            }

            default:
                break;
        }
    }
}

void ZooScan::DeviceOptionState::Updater::SetOption(uint32_t optionIndex, uint32_t valueIndex, bool requestedValue)
{
    auto *option = dynamic_cast<DeviceOptionValue<bool> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
    }

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
        return;
    }

    int optionInfo;
    SANE_Bool saneValue = requestedValue ? SANE_TRUE : SANE_FALSE;
    m_StateComponent->m_Device->SetOptionValue(optionIndex, &saneValue, &optionInfo);

    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        BuildOptions();
        option = dynamic_cast<DeviceOptionValue<bool> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
        }
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        option->SetValue(valueIndex, requestedValue, saneValue);
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

void ZooScan::DeviceOptionState::Updater::SetOption(uint32_t optionIndex, uint32_t valueIndex, int requestedValue)
{
    auto *option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
    }

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
        return;
    }

    auto valueCount = option->ValueCount();
    int optionInfo;
    std::unique_ptr<SANE_Word[]> saneValue(new SANE_Word[valueCount]);
    if (valueCount > 1)
    {
        m_StateComponent->m_Device->GetOptionValue(optionIndex, saneValue.get());
        saneValue[valueIndex] = requestedValue;
    }
    else
    {
        saneValue[0] = requestedValue;
    }

    m_StateComponent->m_Device->SetOptionValue(optionIndex, saneValue.get(), &optionInfo);

    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        BuildOptions();
        option = dynamic_cast<DeviceOptionValue<int> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
        }
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        option->SetValue(valueIndex, requestedValue, saneValue[valueIndex]);
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

void ZooScan::DeviceOptionState::Updater::SetOption(uint32_t optionIndex, uint32_t valueIndex, const std::string& requestedValue)
{
    auto *option = dynamic_cast<DeviceOptionValue<std::string> *>(m_StateComponent->m_OptionValues[optionIndex]);
    if (option == nullptr)
    {
        throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
    }

    if (option->GetRequestedValue(valueIndex) == requestedValue)
    {
        return;
    }

    int optionInfo;
    std::unique_ptr<char[]> saneValue(new char[option->ValueSize() + 1]);
    strcpy(saneValue.get(), requestedValue.c_str());
    m_StateComponent->m_Device->SetOptionValue(optionIndex, saneValue.get(), &optionInfo);

    if ((optionInfo & SANE_INFO_RELOAD_OPTIONS) != 0)
    {
        BuildOptions();
        option = dynamic_cast<DeviceOptionValue<std::string> *>(m_StateComponent->m_OptionValues[optionIndex]);
        if (option == nullptr)
        {
            throw std::runtime_error(std::string("Failed to set option (type mismatch): ") + m_StateComponent->m_OptionValues[optionIndex]->Title());
        }
        option->SetRequestedValue(valueIndex, requestedValue);
    }
    else
    {
        option->SetValue(valueIndex, requestedValue, std::string(saneValue.get()));
        m_StateComponent->GetCurrentChangeset()->AddChangedIndex(WidgetIndex(optionIndex, valueIndex));
    }
}

bool ZooScan::DeviceOptionState::IsPreview() const
{
    auto optionValue = m_OptionValues[m_PreviewIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "preview") == 0))
    {
        return dynamic_cast<DeviceOptionValue<bool>*>(optionValue)->GetValue(0) != 0;
    }

    return false;
}

std::string ZooScan::DeviceOptionState::GetMode() const
{
    auto optionValue = m_OptionValues[m_ModeIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "mode") == 0))
    {
        return dynamic_cast<DeviceOptionValue<std::string>*>(optionValue)->GetValue(0);
    }

    return "";
}

double ZooScan::DeviceOptionState::GetScanAreaX() const
{
    int left = 0;
    SANE_Value_Type valueType;

    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "tl-x") == 0))
    {
        left = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
        valueType = optionValue->ValueType();
        return valueType == SANE_TYPE_FIXED ? SANE_UNFIX(left) : left;
    }

    return 0;
}

double ZooScan::DeviceOptionState::GetScanAreaY() const
{
    int top = 0;
    SANE_Value_Type valueType;

    auto optionValue = m_OptionValues[m_TLYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "tl-y") == 0))
    {
        top = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
        valueType = optionValue->ValueType();
        return valueType == SANE_TYPE_FIXED ? SANE_UNFIX(top) : top;
    }

    return 0;
}

double ZooScan::DeviceOptionState::GetScanAreaWidth() const
{
    int left = 0, right = 0;
    SANE_Value_Type valueType = SANE_TYPE_INT;

    auto optionValue = m_OptionValues[m_TLXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "tl-x") == 0))
    {
        left = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
        valueType = optionValue->ValueType();
    }

    optionValue = m_OptionValues[m_BRXIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "br-x") == 0))
    {
        right = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return valueType == SANE_TYPE_FIXED ? SANE_UNFIX(right - left) : right - left;
}

double ZooScan::DeviceOptionState::GetScanAreaHeight() const
{
    int top = 0, bottom = 0;
    SANE_Value_Type valueType = SANE_TYPE_INT;

    auto optionValue = m_OptionValues[m_TLYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "tl-y") == 0))
    {
        top = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
        valueType = optionValue->ValueType();
    }

    optionValue = m_OptionValues[m_BRYIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "br-y") == 0))
    {
        bottom = dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return valueType == SANE_TYPE_FIXED ? SANE_UNFIX(bottom - top) : bottom - top;
}

int ZooScan::DeviceOptionState::GetResolution() const
{
    auto optionValue = m_OptionValues[m_ResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return 0;
}

int ZooScan::DeviceOptionState::GetXResolution() const
{
    auto optionValue = m_OptionValues[m_XResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "x-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int ZooScan::DeviceOptionState::GetYResolution() const
{
    auto optionValue = m_OptionValues[m_YResolutionIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "y-resolution") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return GetResolution();
}

int ZooScan::DeviceOptionState::GetBitDepth() const
{
    auto optionValue = m_OptionValues[m_BitDepthIndex];
    if (optionValue != nullptr && (strcmp(optionValue->Name(), "depth") == 0))
    {
        return dynamic_cast<DeviceOptionValue<int>*>(optionValue)->GetValue(0);
    }

    return 8;
}
