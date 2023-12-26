#pragma once

#include <sane/sane.h>
#include "DeviceOptionValueBase.h"

namespace ZooScan
{
    template<typename TValueType>
    class DeviceOptionValue : public DeviceOptionValueBase
    {
    public:
        typedef TValueType ValueType;

    private:
        uint32_t m_Size;
        ValueType *m_RequestedValue;
        ValueType *m_Value;

    public:
        explicit DeviceOptionValue(const SANE_Option_Descriptor* optionDescriptor)
                : DeviceOptionValueBase(optionDescriptor)
        {
            m_Size = ValueCount();
            m_RequestedValue = new ValueType[m_Size];
            m_Value = new ValueType[m_Size];
        }

        ~DeviceOptionValue() override
        {
            delete[] m_RequestedValue;
            delete[] m_Value;
            m_Size = 0;
        }

        void SetRequestedValue(uint32_t valueIndex, const ValueType& value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = value;
        }

        void SetValue(uint32_t valueIndex, const ValueType& value)
        {
            SetValue(valueIndex, value, value);
        }

        void SetValue(uint32_t valueIndex, const ValueType& requestedValue, const ValueType& value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        void SetValue(uint32_t valueIndex, const ValueType& requestedValue, ValueType&& value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        [[nodiscard]] const ValueType& GetRequestedValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_RequestedValue[valueIndex];
        }

        [[nodiscard]] const ValueType& GetValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_Value[valueIndex];
        }
    };
}
