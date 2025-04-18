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
            const char *text = defaultText;
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                text = it->second.Title.c_str();
            }

            if (text == nullptr || *text == '\0')
            {
                return text;
            }
            return gettext(text);
        }

        const char *GetDescription(int optionIndex, const char *defaultText)
        {
            const char *text = defaultText;
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                text = it->second.Description.c_str();
            }

            if (text == nullptr || *text == '\0')
            {
                return text;
            }
            return gettext(text);
        }

        void
        GetStringList(int optionIndex, const SANE_String_Const *defaultList, SANE_String_Const *rewrittenStringList)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                auto i = 0UZ;
                for (; i < it->second.StringStringList.size(); ++i)
                {
                    auto text = it->second.StringStringList[i];
                    if (text.empty())
                    {
                        rewrittenStringList[i] = text.c_str();
                    }
                    else
                    {
                        rewrittenStringList[i] = gettext(text.c_str());
                    }
                }
                rewrittenStringList[i] = nullptr;
            }
            else
            {
                auto i = 0UZ;
                for (; defaultList[i] != nullptr; ++i)
                {
                    rewrittenStringList[i] = defaultList[i];
                }
                rewrittenStringList[i] = nullptr;
            }
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
