#pragma once

#include <sane/sane.h>
#include "DeviceOptionValueBase.hpp"

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
        explicit DeviceOptionValue(const SANE_Option_Descriptor *optionDescriptor)
            : DeviceOptionValueBase(optionDescriptor)
        {
            m_Size = GetValueCount();
            m_RequestedValue = new ValueType[m_Size];
            m_Value = new ValueType[m_Size];
        }

        ~DeviceOptionValue() override
        {
            delete[] m_RequestedValue;
            delete[] m_Value;
            m_Size = 0;
        }

        void Serialize(nlohmann::json &parentObject) const override
        {
            auto values = nlohmann::json::array();
            for (auto i = 0U; i < m_Size; i++)
            {
                values.push_back(m_RequestedValue[i]);
            }
            parentObject[GetName()] = values;
        }

        bool Deserialize(const nlohmann::json &parentObject) override
        {
            bool updated = false;
            if (parentObject.contains(GetName()))
            {
                auto values = parentObject[GetName()];
                if (values.is_array())
                {
                    std::unique_ptr<ValueType[]> newValues(new ValueType[values.size()]);

                    for (auto i = 0UL; i < values.size(); i++)
                    {
                        if (m_RequestedValue[i] == values[i].template get<ValueType>())
                        {
                            newValues[i] = values[i];
                            updated = true;
                        }
                        else
                        {
                            newValues[i] = m_RequestedValue[i];
                        }
                    }

                    if (updated)
                    {
                        delete[] m_RequestedValue;
                        delete[] m_Value;

                        m_Size = values.size();
                        m_Value = new ValueType[m_Size];
                        m_RequestedValue = newValues.release();
                    }
                }
            }

            return updated;
            ;
        }

        void SetRequestedValue(uint32_t valueIndex, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = value;
        }

        void SetDeviceValue(uint32_t valueIndex, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_Value[valueIndex] = value;
        }

        void SetValues(uint32_t valueIndex, const ValueType &requestedValue, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        void SetValues(uint32_t valueIndex, const ValueType &requestedValue, ValueType &&value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        [[nodiscard]] const ValueType &GetRequestedValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_RequestedValue[valueIndex];
        }

        [[nodiscard]] const ValueType &GetValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_Value[valueIndex];
        }
    };
}
