#pragma once

#include <sane/sane.h>
#include "DeviceSettingsValueBase.h"

namespace ZooScan
{
    template<typename TValueType>
    class DeviceSettingValue : public DeviceSettingValueBase
    {
    public:
        typedef TValueType ValueType;

    private:
        ValueType *m_RequestedValue;
        ValueType *m_Value;

    public:
        explicit DeviceSettingValue(SANE_Value_Type valueType, int valueCount = 1)
                : DeviceSettingValueBase(valueType, valueCount)
        {
            m_RequestedValue = new ValueType[valueCount];
            m_Value = new ValueType[valueCount];
        }

        ~DeviceSettingValue() override
        {
            delete[] m_RequestedValue;
            delete[] m_Value;
        }

        void SetValue(uint32_t valueIndex, const ValueType& value)
        {
            SetValue(valueIndex, value, value);
        }

        void SetValue(uint32_t valueIndex, const ValueType& requestedValue, const ValueType& value)
        {
            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        [[nodiscard]] const ValueType& GetValue(uint32_t valueIndex = 0) const
        {
            return m_Value[valueIndex];
        }
    };
}
