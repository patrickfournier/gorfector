#pragma once

#include <sane/sane.h>

#include "DeviceOptionValueBase.hpp"

namespace Gorfector
{
    /**
     * \brief Template class for representing a device option value.
     *
     * This class extends `DeviceOptionValueBase` and provides functionality for managing
     * device option values of a specific type, including serialization, deserialization,
     * and value manipulation.
     *
     * \tparam TValueType The type of the device option value.
     */
    template<typename TValueType>
    class DeviceOptionValue final : public DeviceOptionValueBase
    {
    public:
        /// Alias for the value type.
        typedef TValueType ValueType;

    private:
        /// The number of values for this option.
        uint32_t m_Size;

        /// Pointer to the requested values for the option.
        ValueType *m_RequestedValue;

        /// Pointer to the actual values for the option.
        ValueType *m_Value;

    public:
        /**
         * \brief Constructs a `DeviceOptionValue` object.
         *
         * \param optionDescriptor Pointer to the SANE option descriptor.
         */
        explicit DeviceOptionValue(const SANE_Option_Descriptor *optionDescriptor)
            : DeviceOptionValueBase(optionDescriptor)
        {
            m_Size = GetValueCount();
            m_RequestedValue = new ValueType[m_Size];
            m_Value = new ValueType[m_Size];
        }

        /**
         * \brief Destructor for `DeviceOptionValue`.
         *
         * Cleans up dynamically allocated memory for the requested and actual values.
         */
        ~DeviceOptionValue() override
        {
            delete[] m_RequestedValue;
            delete[] m_Value;
            m_Size = 0;
        }

        /**
         * \brief Serializes the requested values into a JSON object.
         *
         * \param parentObject Reference to the JSON object where the values will be serialized.
         */
        void Serialize(nlohmann::json &parentObject) const override
        {
            auto values = nlohmann::json::array();
            for (auto i = 0U; i < m_Size; i++)
            {
                values.push_back(m_RequestedValue[i]);
            }
            parentObject[GetName()] = values;
        }

        /**
         * \brief Deserializes the requested values from a JSON object.
         *
         * \param parentObject The JSON object containing the serialized values.
         */
        void Deserialize(const nlohmann::json &parentObject) override
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
                        if (m_RequestedValue[i] != values[i].template get<ValueType>())
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

                        m_Size = values.size();
                        m_RequestedValue = newValues.release();
                    }
                }
            }
        }

        /**
         * \brief Sets the requested value at a specific index.
         *
         * \param valueIndex The index of the value to set.
         * \param value The requested value to set.
         * \throws std::out_of_range If the index is out of bounds.
         */
        void SetRequestedValue(uint32_t valueIndex, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = value;
        }

        /**
         * \brief Sets the actual device value at a specific index.
         *
         * \param valueIndex The index of the value to set.
         * \param value The actual value to set.
         * \throws std::out_of_range If the index is out of bounds.
         */
        void SetDeviceValue(uint32_t valueIndex, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_Value[valueIndex] = value;
        }

        /**
         * \brief Sets both the requested and actual values at a specific index.
         *
         * \param valueIndex The index of the value to set.
         * \param requestedValue The requested value to set.
         * \param value The actual value to set.
         * \throws std::out_of_range If the index is out of bounds.
         */
        void SetValues(uint32_t valueIndex, const ValueType &requestedValue, const ValueType &value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        /**
         * \brief Sets both the requested and actual values at a specific index using move semantics.
         *
         * \param valueIndex The index of the value to set.
         * \param requestedValue The requested value to set.
         * \param value The actual value to set (rvalue reference).
         * \throws std::out_of_range If the index is out of bounds.
         */
        void SetValues(uint32_t valueIndex, const ValueType &requestedValue, ValueType &&value)
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            m_RequestedValue[valueIndex] = requestedValue;
            m_Value[valueIndex] = value;
        }

        /**
         * \brief Gets the requested value at a specific index.
         *
         * \param valueIndex The index of the value to retrieve (default is 0).
         * \return The requested value at the specified index.
         * \throws std::out_of_range If the index is out of bounds.
         */
        [[nodiscard]] const ValueType &GetRequestedValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_RequestedValue[valueIndex];
        }

        /**
         * \brief Gets the actual device value at a specific index.
         *
         * \param valueIndex The index of the value to retrieve (default is 0).
         * \return The actual value at the specified index.
         * \throws std::out_of_range If the index is out of bounds.
         */
        [[nodiscard]] const ValueType &GetValue(uint32_t valueIndex = 0) const
        {
            if (valueIndex >= m_Size)
                throw std::out_of_range("valueIndex");

            return m_Value[valueIndex];
        }
    };
}
