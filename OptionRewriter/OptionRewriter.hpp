#pragma once

#include <cstdint>
#include <map>
#include <sane/sane.h>
#include <sstream>
#include <string>
#include <vector>

#include "SaneDevice.hpp"

namespace ZooScan
{
    class DeviceOptionsState;
    class DeviceOptionValueBase;

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
        const char *Title;
        const char *Description;
        std::vector<const char *> StringList;
        uint32_t Flags;
    };

    class OptionRewriter
    {
    public:
        static void Dump(SaneDevice *device);

        static std::string ToTitleCase(const std::string &text)
        {
            std::stringstream result;
            bool newWord = true;

            for (const auto ch: text)
            {
                newWord = newWord || std::isspace(ch);
                if (std::isalpha(ch))
                {
                    if (newWord)
                    {
                        result << static_cast<char>(std::toupper(ch));
                        newWord = false;
                    }
                    else
                    {
                        result << static_cast<char>(std::tolower(ch));
                    }
                }
                else
                {
                    result << ch;
                }
            }

            return result.str();
        }

    protected:
        [[nodiscard]] virtual const std::map<std::string, std::vector<std::string>> &GetSupportedDevices() const = 0;

    public:
        virtual ~OptionRewriter() = default;

        [[nodiscard]] std::vector<std::string> GetSupportedVendors() const
        {
            auto deviceMap = GetSupportedDevices();
            std::vector<std::string> vendors;
            vendors.reserve(deviceMap.size());
            for (const auto &[vendor, _]: deviceMap)
            {
                vendors.push_back(vendor);
            }

            return vendors;
        }

        [[nodiscard]] const std::vector<std::string> &GetSupportedModels(const std::string &vendor) const
        {
            auto deviceMap = GetSupportedDevices();
            auto it = deviceMap.find(vendor);
            if (it != deviceMap.end())
            {
                return it->second;
            }

            static const std::vector<std::string> empty;
            return empty;
        }

        virtual const char *GetTitle(int optionIndex, const char *defaultText)
        {
            return defaultText;
        }

        virtual const char *GetDescription(int optionIndex, const char *defaultText)
        {
            return defaultText;
        }

        virtual const SANE_String_Const *GetStringList(int optionIndex, const SANE_String_Const *defaultList)
        {
            return defaultList;
        }

        virtual bool IsDisplayOnly(int optionIndex, bool defaultValue)
        {
            return defaultValue;
        }

        virtual bool ShouldHide(int optionIndex, bool defaultValue)
        {
            return defaultValue;
        }

        virtual bool IsAdvanced(int optionIndex, bool defaultValue)
        {
            return defaultValue;
        }
    };
}
