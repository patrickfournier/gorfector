#pragma once
#include "OptionRewriter.hpp"

#include <cstring>
#include <sstream>

namespace ZooScan
{
    class EpsonPerfectionV600PhotoRewriter : public OptionRewriter
    {
        std::map<std::string, std::vector<std::string>> m_SupportedDevices{
                {"Epson", {"Perfection V600 Photo"}},
        };

        static const std::map<uint32_t, OptionInfos> k_OptionInfos;

    protected:
        [[nodiscard]] const std::map<std::string, std::vector<std::string>> &GetSupportedDevices() const override
        {
            return m_SupportedDevices;
        }

    public:
        const char *GetTitle(int optionIndex, const char *defaultText) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                return it->second.Title;
            }
            return defaultText;
        }

        const char *GetDescription(int optionIndex, const char *defaultText) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                return it->second.Description;
            }
            return defaultText;
        }

        const SANE_String_Const *GetStringList(int optionIndex, const SANE_String_Const *defaultList) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                return it->second.StringList.data();
            }
            return defaultList;
        }

        bool IsDisplayOnly(int optionIndex, bool defaultValue) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                return it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceReadOnly);
            }

            return defaultValue;
        }

        bool ShouldHide(int optionIndex, bool defaultValue) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                if (it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceHidden))
                {
                    return true;
                }

                if (it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceShown))
                {
                    return false;
                }
            }

            return defaultValue;
        }

        bool IsAdvanced(int optionIndex, bool defaultValue) override
        {
            if (auto it = k_OptionInfos.find(optionIndex); it != k_OptionInfos.end())
            {
                if (it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceAdvanced))
                {
                    return true;
                }

                if (it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceBasic))
                {
                    return false;
                }
            }

            return defaultValue;
        }
    };
}
