#pragma once

#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <sane/sane.h>
#include <string>
#include <vector>

#include "SaneDevice.hpp"

namespace Gorfector
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
        for (const auto &flag: j["flags"])
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
        /**
         * \brief Stores information about options, indexed by their option index.
         */
        std::map<uint32_t, OptionInfos> m_OptionInfos;

    public:
        /**
         * \brief Dumps the options of a given SANE device.
         * \param device Pointer to the SANE device whose options are to be dumped.
         */
        static void Dump(SaneDevice *device);

        /**
         * \brief Constructs an `OptionRewriter` instance.
         * \param systemConfigPath Path to system config files.
         * \param userConfigPath Path to user config files.
         */
        OptionRewriter(const std::filesystem::path &systemConfigPath, const std::filesystem::path &userConfigPath);

        /**
         * \brief Default destructor for the `OptionRewriter` class.
         */
        ~OptionRewriter() = default;

        /**
         * \brief Loads option descriptions for a specific device based on its vendor and model.
         * \param systemConfigPath Path to system configuration files.
         * \param userConfigPath Path to user configuration files.
         * \param deviceVendor The vendor name of the device.
         * \param deviceModel The model name of the device.
         */
        void LoadOptionDescriptionFile(
                const std::filesystem::path &systemConfigPath, const std::filesystem::path &userConfigPath,
                const char *deviceVendor, const char *deviceModel);

        /**
         * \brief Retrieves the title of an option.
         * \param optionIndex The index of the option.
         * \param defaultText The default title to return if no custom title is found.
         * \return The title of the option, or the default text if no custom title is available.
         */
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

        /**
         * \brief Retrieves the description of an option.
         * \param optionIndex The index of the option.
         * \param defaultText The default description to return if no custom description is found.
         * \return The description of the option, or the default text if no custom description is available.
         */
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

        /**
         * \brief Retrieves a rewritten string list for an option.
         * \param optionIndex The index of the option.
         * \param defaultList The default string list to use if no custom list is found.
         * \param rewrittenStringList The output parameter to store the rewritten string list.
         */
        void
        GetStringList(int optionIndex, const SANE_String_Const *defaultList, SANE_String_Const *rewrittenStringList)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                auto i = 0UZ;
                for (; i < it->second.StringStringList.size(); ++i)
                {
                    auto &text = it->second.StringStringList[i];
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
                    if (defaultList[i][0] == '\0')
                    {
                        rewrittenStringList[i] = defaultList[i];
                    }
                    else
                    {
                        rewrittenStringList[i] = gettext(defaultList[i]);
                    }
                }
                rewrittenStringList[i] = nullptr;
            }
        }

        /**
         * \brief Checks if an option is display-only (read-only).
         * \param optionIndex The index of the option.
         * \param defaultValue The default value to return if no custom flag is found.
         * \return True if the option is display-only, otherwise false.
         */
        bool IsDisplayOnly(int optionIndex, bool defaultValue)
        {
            if (auto it = m_OptionInfos.find(optionIndex); it != m_OptionInfos.end())
            {
                return it->second.Flags & static_cast<uint32_t>(OptionFlags::e_ForceReadOnly);
            }

            return defaultValue;
        }

        /**
         * \brief Checks if an option should be hidden.
         * \param optionIndex The index of the option.
         * \param defaultValue The default value to return if no custom flag is found.
         * \return True if the option should be hidden, otherwise false.
         */
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

        /**
         * \brief Checks if an option is considered advanced.
         * \param optionIndex The index of the option.
         * \param defaultValue The default value to return if no custom flag is found.
         * \return True if the option is advanced, otherwise false.
         */
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
