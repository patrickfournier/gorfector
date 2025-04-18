#pragma once

#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <sane/sane.h>
#include <string>
#include <vector>

#include "SaneDevice.hpp"

namespace ZooScan
{
    enum class OptionFlags : uint32_t
    {
        e_NoChange = 0,
        e_ForceBasic = 1 << 0,
        e_ForceAdvanced = 1 << 1,
        e_ForceShown = 1 << 2,
        e_ForceHidden = 1 << 3,
        e_ForceReadOnly = 1 << 4,
    };

    struct OptionInfos
    {
        std::string Title;
        std::string Description;
        std::vector<std::string> StringStringList;
        std::vector<const char *> StringList;
        uint32_t Flags;
    };

    inline void from_json(const nlohmann::json &j, OptionInfos &p)
    {
        j.at("title").get_to(p.Title);
        j.at("description").get_to(p.Description);
        j.at("string_list").get_to(p.StringStringList);

        uint32_t flags{};
        for (auto flag: j["flags"])
        {
            if (flag == "ForceBasic")
            {
                flags |= static_cast<uint32_t>(OptionFlags::e_ForceBasic);
            }
            else if (flag == "ForceAdvanced")
            {
                flags |= static_cast<uint32_t>(OptionFlags::e_ForceAdvanced);
            }
            else if (flag == "ForceShown")
            {
                flags |= static_cast<uint32_t>(OptionFlags::e_ForceShown);
            }
            else if (flag == "ForceHidden")
            {
                flags |= static_cast<uint32_t>(OptionFlags::e_ForceHidden);
            }
            else if (flag == "ForceReadOnly")
            {
                flags |= static_cast<uint32_t>(OptionFlags::e_ForceReadOnly);
            }
        }
        p.Flags = flags;
    }

    class OptionRewriter final
    {
        std::map<uint32_t, OptionInfos> m_OptionInfos;

    public:
        static void Dump(SaneDevice *device);

        OptionRewriter();
        ~OptionRewriter() = default;

        void LoadOptionDescriptionFile(const char *deviceVendor, const char *deviceModel);

        const char *GetTitle(int optionIndex, const char *defaultText)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                return it->second.Title.c_str();
            }
            return defaultText;
        }

        const char *GetDescription(int optionIndex, const char *defaultText)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                return it->second.Description.c_str();
            }
            return defaultText;
        }

        const SANE_String_Const *GetStringList(int optionIndex, const SANE_String_Const *defaultList)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                if (!it->second.StringStringList.empty() &&
                    it->second.StringList.size() != it->second.StringStringList.size() + 1)
                {
                    it->second.StringList.clear();

                    it->second.StringList.reserve(it->second.StringStringList.size() + 1);
                    for (auto &str: it->second.StringStringList)
                    {
                        it->second.StringList.push_back(str.c_str());
                    }
                    it->second.StringList.push_back(nullptr);
                }

                return it->second.StringList.data();
            }
            return defaultList;
        }

        bool IsDisplayOnly(int optionIndex, bool defaultValue)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                return it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceReadOnly);
            }

            return defaultValue;
        }

        bool ShouldHide(int optionIndex, bool defaultValue)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
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

        bool IsAdvanced(int optionIndex, bool defaultValue)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
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
