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
        ValueType *m_Value;

    public:
        DeviceSettingValue(int settingIndex, SANE_Value_Type valueType, int valueCount = 1)
                : DeviceSettingValueBase(settingIndex, valueType)
        {
            m_Value = new ValueType[valueCount];
        }

        ~DeviceSettingValue() override
        {
            delete[] m_Value;
        }

        void SetValue(int valueIndex, ValueType value)
        {
            m_Value[valueIndex] = value;
        }

        [[nodiscard]] ValueType GetValue(int valueIndex = 0) const
        {
            return m_Value[valueIndex];
        }
    };
}
