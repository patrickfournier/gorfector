#include "OptionRewriter.hpp"

#include <DeviceOptionsState.hpp>
#include <glib.h>

void ZooScan::OptionRewriter::Dump(SaneDevice *device)
{
    g_print("Vendor: %s\n", device->GetVendor());
    g_print("Model: %s\n", device->GetModel());

    SANE_Int optionCount;
    device->GetOptionValue(0, &optionCount);
    for (auto i = 0; i < optionCount; i++)
    {
        auto option = device->GetOptionDescriptor(i);
        if (option == nullptr)
        {
            continue;
        }

        auto flags = std::vector<std::string>{};
        if (SaneDevice::IsAdvanced(*option))
        {
            flags.emplace_back("advanced");
        }
        if (SaneDevice::IsDisplayOnly(*option))
        {
            flags.emplace_back("display only");
        }

        std::string description{};
        if (option->desc != nullptr)
        {
            auto descLen = std::strlen(option->desc);
            description.reserve(descLen);
            auto n = 0UZ;
            while (n < descLen)
            {
                if (option->desc[n] == '"')
                {
                    description += "\\\"";
                }
                else
                {
                    description += option->desc[n];
                }
                n++;
            }
        }

        auto flagsStr = std::string{};
        if (!flags.empty())
        {
            flagsStr = flags[0];
            for (auto j = 1UZ; j < flags.size(); j++)
            {
                flagsStr += ", ";
                flagsStr += flags[j];
            }
        }

        auto stringListStr = std::string{};

        if (option->type == SANE_TYPE_STRING && option->constraint_type == SANE_CONSTRAINT_STRING_LIST)
        {
            auto stringList = option->constraint.string_list;
            if (stringList != nullptr && stringList[0] != nullptr)
            {
                stringListStr = "\"";
                stringListStr += stringList[0];
                stringListStr += "\"";

                for (auto j = 1; stringList[j] != nullptr; j++)
                {
                    stringListStr += ", \"";
                    stringListStr += stringList[j];
                    stringListStr += "\"";
                }

                stringListStr += ", nullptr";
            }
        }

        auto title = option->title == nullptr ? "" : option->title;
        auto name = option->name == nullptr ? "<no name>" : option->name;
        std::string comment = " // ";
        comment += name;
        comment += " (";
        comment += flagsStr;
        comment += ")";

        g_print("{%d, {\"%s\", \"%s\", {%s}, 0x0}},%s\n", i, title, description.c_str(), stringListStr.c_str(),
                comment.c_str());
    }
}
