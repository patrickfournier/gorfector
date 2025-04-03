#pragma once

#include <sane/sane.h>
#include "nlohmann_json/json.hpp"

namespace ZooScan
{
    class DeviceOptionValueBase
    {
        const SANE_Option_Descriptor *m_OptionDescriptor;

    public:
        explicit DeviceOptionValueBase(const SANE_Option_Descriptor *optionDescriptor)
            : m_OptionDescriptor(optionDescriptor)
        {
        }

        virtual ~DeviceOptionValueBase() = default;

        virtual void Serialize(nlohmann::json &parent) const = 0;
        virtual bool Deserialize(const nlohmann::json &json) = 0;

        [[nodiscard]] const char *GetName() const
        {
            return m_OptionDescriptor->name;
        }

        [[nodiscard]] const char *GetTitle() const
        {
            return m_OptionDescriptor->title;
        }

        [[nodiscard]] const char *GetDescription() const
        {
            return m_OptionDescriptor->desc;
        }

        [[nodiscard]] SANE_Unit GetUnit() const
        {
            return m_OptionDescriptor->unit;
        }

        [[nodiscard]] SANE_Value_Type GetValueType() const
        {
            return m_OptionDescriptor->type;
        }

        [[nodiscard]] bool IsRange() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_RANGE &&
                   m_OptionDescriptor->constraint.range != nullptr;
        }

        [[nodiscard]] const SANE_Range *GetRange() const
        {
            if (IsRange())
                return m_OptionDescriptor->constraint.range;

            return nullptr;
        }

        [[nodiscard]] bool IsNumberList() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_WORD_LIST &&
                   m_OptionDescriptor->constraint.word_list != nullptr &&
                   m_OptionDescriptor->constraint.word_list[0] > 0;
        }

        [[nodiscard]] const SANE_Word *GetNumberList() const
        {
            if (IsNumberList())
                return m_OptionDescriptor->constraint.word_list;

            return nullptr;
        }

        [[nodiscard]] bool IsStringList() const
        {
            return m_OptionDescriptor->constraint_type == SANE_CONSTRAINT_STRING_LIST &&
                   m_OptionDescriptor->constraint.string_list != nullptr;
        }

        [[nodiscard]] const SANE_String_Const *GetStringList() const
        {
            if (IsStringList())
                return m_OptionDescriptor->constraint.string_list;

            return nullptr;
        }

        [[nodiscard]] unsigned long GetValueCount() const
        {
            auto type = m_OptionDescriptor->type;
            if (type == SANE_TYPE_BOOL || type == SANE_TYPE_INT || type == SANE_TYPE_FIXED)
            {
                return m_OptionDescriptor->size / sizeof(SANE_Word);
            }

            return 1;
        }

        [[nodiscard]] unsigned long GetValueSize() const
        {
            return m_OptionDescriptor->size;
        }

        [[nodiscard]] bool IsDisplayOnly() const
        {
            return DeviceOptionValueBase::IsDisplayOnly(*m_OptionDescriptor);
        }

        [[nodiscard]] bool ShouldHide() const
        {
            return DeviceOptionValueBase::ShouldHide(*m_OptionDescriptor);
        }

        [[nodiscard]] bool IsSoftwareSettable() const
        {
            return DeviceOptionValueBase::IsSoftwareSettable(*m_OptionDescriptor);
        }

        [[nodiscard]] bool IsAdvanced() const
        {
            return DeviceOptionValueBase::IsAdvanced(*m_OptionDescriptor);
        }

        [[nodiscard]] static bool IsDisplayOnly(const SANE_Option_Descriptor &optionDescriptor)
        {
            auto caps = optionDescriptor.cap;
            return (caps & SANE_CAP_SOFT_DETECT) && !(caps & SANE_CAP_SOFT_SELECT) && !(caps & SANE_CAP_HARD_SELECT);
        }

        [[nodiscard]] static bool ShouldHide(const SANE_Option_Descriptor &optionDescriptor)
        {
            auto caps = optionDescriptor.cap;
            bool notSettable = (caps & SANE_CAP_INACTIVE) || (caps & SANE_CAP_HARD_SELECT);

            if (notSettable)
                return true;

            bool hideBecauseOfTitle =
                    optionDescriptor.title != nullptr && (strcasestr(optionDescriptor.title, "deprecat") != nullptr ||
                                                          strcasestr(optionDescriptor.title, "preview") != nullptr);

            if (hideBecauseOfTitle)
                return true;

            bool hideBecauseOfDesc =
                    optionDescriptor.desc != nullptr && (strcasestr(optionDescriptor.desc, "deprecat") != nullptr ||
                                                         strcasestr(optionDescriptor.desc, "preview") != nullptr);

            return hideBecauseOfDesc;
        }

        [[nodiscard]] static bool IsSoftwareSettable(const SANE_Option_Descriptor &optionDescriptor)
        {
            return (optionDescriptor.cap & SANE_CAP_SOFT_SELECT);
        }

        [[nodiscard]] static bool IsAdvanced(const SANE_Option_Descriptor &optionDescriptor)
        {
            return (optionDescriptor.cap & SANE_CAP_ADVANCED);
        }
    };
}
