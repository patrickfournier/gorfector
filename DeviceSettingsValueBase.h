#pragma once

#include <sane/sane.h>

namespace ZooScan
{
    class DeviceSettingValueBase
    {
        SANE_Value_Type m_ValueType;
        int m_ValueCount;

    public:
        explicit DeviceSettingValueBase(SANE_Value_Type valueType, int valueCount = 1)
        {
            m_ValueType = valueType;
            m_ValueCount = valueCount;
        }

        virtual ~DeviceSettingValueBase() = default;

        [[nodiscard]] SANE_Value_Type ValueType() const
        { return m_ValueType; }

        [[nodiscard]] int ValueCount() const
        {
            return m_ValueCount;
        }
    };
}
