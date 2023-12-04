#pragma once

#include <sane/sane.h>

namespace ZooScan
{
    class DeviceSettingValueBase
    {
        int m_SettingIndex;
        SANE_Value_Type m_ValueType;

    public:
        DeviceSettingValueBase(int settingIndex, SANE_Value_Type valueType)
        {
            m_SettingIndex = settingIndex;
            m_ValueType = valueType;
        }

        virtual ~DeviceSettingValueBase() = default;

        [[nodiscard]] int SettingIndex() const
        { return m_SettingIndex; }

        [[nodiscard]] SANE_Value_Type ValueType() const
        { return m_ValueType; }
    };
}
