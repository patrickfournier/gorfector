#pragma once

#include <nlohmann/json.hpp>
#include <sane/sane.h>

#include "SaneDevice.hpp"

namespace Gorfector
{
    /**
     * \brief Base class for representing a device option value.
     *
     * This class provides an interface for handling device option values, including serialization,
     * deserialization, and retrieving metadata about the option.
     */
    class DeviceOptionValueBase
    {
        /// Pointer to the SANE option descriptor for this device option.
        const SANE_Option_Descriptor *m_OptionDescriptor;

    public:
        /**
         * \brief Constructs a DeviceOptionValueBase.
         *
         * \param optionDescriptor Pointer to the SANE option descriptor.
         */
        explicit DeviceOptionValueBase(const SANE_Option_Descriptor *optionDescriptor)
            : m_OptionDescriptor(optionDescriptor)
        {
        }

        /// Virtual destructor.
        virtual ~DeviceOptionValueBase() = default;

        /**
         * \brief Serializes the device option value into a JSON object.
         *
         * \param parent Reference to the JSON object where the value will be serialized.
         */
        virtual void Serialize(nlohmann::json &parent) const = 0;

        /**
         * \brief Deserializes the device option value from a JSON object.
         *
         * \param json The JSON object containing the serialized value.
         */
        virtual void Deserialize(const nlohmann::json &json) = 0;

        /**
         * \brief Gets the name of the option.
         *
         * \return The name of the option.
         */
        [[nodiscard]] const char *GetName() const
        {
            return m_OptionDescriptor->name;
        }

        /**
         * \brief Gets the title of the option.
         *
         * \return The title of the option.
         */
        [[nodiscard]] const char *GetTitle() const
        {
            return m_OptionDescriptor->title;
        }

        /**
         * \brief Gets the description of the option.
         *
         * \return The description of the option.
         */
        [[nodiscard]] const char *GetDescription() const
        {
            return m_OptionDescriptor->desc;
        }

        /**
         * \brief Gets the unit of the option value.
         *
         * \return The unit of the option value.
         */
        [[nodiscard]] SANE_Unit GetUnit() const
        {
            return m_OptionDescriptor->unit;
        }

        /**
         * \brief Gets the type of the option value.
         *
         * \return The type of the option value.
         */
        [[nodiscard]] SANE_Value_Type GetValueType() const
        {
            return m_OptionDescriptor->type;
        }

        /**
         * \brief Checks if the option value is constrained by a range.
         *
         * \return True if the option value is a range, false otherwise.
         */
        [[nodiscard]] bool IsRange() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_RANGE &&
                   m_OptionDescriptor->constraint.range != nullptr;
        }

        /**
         * \brief Gets the range constraint of the option value if applicable.
         *
         * \return Pointer to the range constraint, or nullptr if not applicable.
         */
        [[nodiscard]] const SANE_Range *GetRange() const
        {
            if (IsRange())
                return m_OptionDescriptor->constraint.range;

            return nullptr;
        }

        /**
         * \brief Checks if the option value is constrained by a list of numbers.
         *
         * \return True if the option value is constrained by a number list, false otherwise.
         */
        [[nodiscard]] bool IsNumberList() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_WORD_LIST &&
                   m_OptionDescriptor->constraint.word_list != nullptr &&
                   m_OptionDescriptor->constraint.word_list[0] > 0;
        }

        /**
         * \brief Gets the list of valid numbers for the option value if applicable.
         *
         * \return Pointer to the number list, or nullptr if not applicable.
         */
        [[nodiscard]] const SANE_Word *GetNumberList() const
        {
            if (IsNumberList())
                return m_OptionDescriptor->constraint.word_list;

            return nullptr;
        }

        /**
         * \brief Checks if the option value is constrained by a list of strings.
         *
         * \return True if the option value is constrained by a string list, false otherwise.
         */
        [[nodiscard]] bool IsStringList() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_STRING_LIST &&
                   m_OptionDescriptor->constraint.string_list != nullptr;
        }

        /**
         * \brief Gets the list of strings for the option value if applicable.
         *
         * \return Pointer to the string list, or nullptr if not applicable.
         */
        [[nodiscard]] const SANE_String_Const *GetStringList() const
        {
            if (IsStringList())
            {
                return m_OptionDescriptor->constraint.string_list;
            }

            return nullptr;
        }

        /**
         * \brief Gets the number of values for the option.
         *
         * \return The number of values.
         */
        [[nodiscard]] unsigned long GetValueCount() const
        {
            auto type = m_OptionDescriptor->type;
            if (type == SANE_TYPE_BOOL || type == SANE_TYPE_INT || type == SANE_TYPE_FIXED)
            {
                return m_OptionDescriptor->size / sizeof(SANE_Word);
            }

            return 1;
        }

        /**
         * \brief Gets the size of the option value.
         *
         * \return The size of the option value in bytes.
         */
        [[nodiscard]] unsigned long GetValueSize() const
        {
            return m_OptionDescriptor->size;
        }

        /**
         * \brief Checks if the option is display-only.
         *
         * \return True if the option is display-only, false otherwise.
         */
        [[nodiscard]] bool IsDisplayOnly() const
        {
            return SaneDevice::IsDisplayOnly(*m_OptionDescriptor);
        }

        /**
         * \brief Checks if the option should be hidden.
         *
         * \return True if the option should be hidden, false otherwise.
         */
        [[nodiscard]] bool ShouldHide() const
        {
            return SaneDevice::ShouldHide(*m_OptionDescriptor);
        }

        /**
         * \brief Checks if the option is software-settable.
         *
         * \return True if the option is software-settable, false otherwise.
         */
        [[nodiscard]] bool IsSoftwareSettable() const
        {
            return SaneDevice::IsSoftwareSettable(*m_OptionDescriptor);
        }

        /**
         * \brief Checks if the option is considered advanced.
         *
         * \return True if the option is advanced, false otherwise.
         */
        [[nodiscard]] bool IsAdvanced() const
        {
            return SaneDevice::IsAdvanced(*m_OptionDescriptor);
        }
    };
}
