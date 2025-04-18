#include "OptionRewriter.hpp"

#include <format>
#include <glib.h>

#include "App.hpp"
#include "DeviceOptionsState.hpp"

void ZooScan::OptionRewriter::Dump(SaneDevice *device)
{
    auto deviceInfo = nlohmann::json{};

    deviceInfo["devices"] = nlohmann::json::array();

    auto deviceJson = nlohmann::json({{"vendor", device->GetVendor()}, {"model", device->GetModel()}});
    deviceInfo["devices"].push_back(deviceJson);

    SANE_Int optionCount;
    device->GetOptionValue(0, &optionCount);
    for (auto i = 1; i < optionCount; i++)
    {
        auto option = device->GetOptionDescriptor(i);
        if (option == nullptr)
        {
            continue;
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

        auto stringListJson = nlohmann::json::array();
        if (option->type == SANE_TYPE_STRING && option->constraint_type == SANE_CONSTRAINT_STRING_LIST)
        {
            auto stringList = option->constraint.string_list;
            if (stringList != nullptr && stringList[0] != nullptr)
            {
                for (auto j = 0; stringList[j] != nullptr; j++)
                {
                    stringListJson.push_back(stringList[j]);
                }
            }
        }

        auto title = option->title == nullptr ? "" : option->title;

        auto name = option->name == nullptr ? "<no name>" : option->name;

        auto flags = std::vector<std::string>{};
        if (SaneDevice::IsAdvanced(*option))
        {
            flags.emplace_back("advanced");
        }
        if (SaneDevice::IsDisplayOnly(*option))
        {
            flags.emplace_back("display only");
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
        std::string comment{};
        comment += name;
        comment += " (";
        comment += flagsStr;
        comment += ")";

        auto jsonOption = nlohmann::json();
        jsonOption["id"] = i;
        jsonOption["title"] = title;
        jsonOption["description"] = description;
        jsonOption["string_list"] = stringListJson;
        jsonOption["flags"] = nlohmann::json::array();
        jsonOption["comment"] = comment;

        deviceInfo["options"].push_back(jsonOption);

        auto optionDump = deviceInfo["options"].dump();
        auto hash = std::hash<std::string>{}(optionDump);
        deviceInfo["option_hash"] = std::format("{:x}", hash);
    }

    g_print("%s", deviceInfo.dump(4).c_str());
}

void AddToIndex(
        nlohmann::json &index, const std::string &vendorName, const std::string &modelName, const std::string &filePath)
{
    for (auto vendor: index["contents"])
    {
        if (vendor["vendor"] == vendorName)
        {
            for (auto model: vendor["models"])
            {
                if (model["name"] == modelName)
                {
                    model["filepath"] = filePath;
                }
            }

            auto newModel = nlohmann::json({{"name", modelName}, {"filepath", filePath}});
            vendor["models"].push_back(newModel);
            return;
        }
    }

    auto newVendor = nlohmann::json({{"vendor", vendorName}, {"models", nlohmann::json::array()}});
    auto newModel = nlohmann::json({{"name", modelName}, {"filepath", filePath}});
    newVendor["models"].push_back(newModel);
    index["contents"].push_back(newVendor);
}

void UpdateIndexRecursively(
        const std::filesystem::path &scannersPath, const std::filesystem::file_time_type &indexModifiedTime,
        nlohmann::json &index)
{
    if (!std::filesystem::exists(scannersPath))
    {
        return;
    }

    for (const auto &entry: std::filesystem::directory_iterator(scannersPath))
    {
        if (entry.is_directory())
        {
            UpdateIndexRecursively(entry.path(), indexModifiedTime, index);
        }
        else if (entry.path().extension() == ".json")
        {
            auto scannerFilePath = entry.path();
            if (std::filesystem::exists(scannerFilePath) &&
                std::filesystem::last_write_time(scannerFilePath) > indexModifiedTime)
            {
                std::ifstream scannerFile(scannerFilePath);
                auto scannerData = nlohmann::json::parse(scannerFile);
                scannerFile.close();

                if (scannerData.is_null() || !scannerData.contains("devices") || !scannerData["devices"].is_array())
                {
                    continue;
                }

                for (auto device: scannerData["devices"])
                {
                    auto vendor = device["vendor"].get<std::string>();
                    auto model = device["model"].get<std::string>();
                    AddToIndex(index, vendor, model, canonical(scannerFilePath).string());
                }
            }
        }
    }
}

ZooScan::OptionRewriter::OptionRewriter()
{
    auto baseDirectory = std::filesystem::current_path() / "../scanners";
    auto prefDirectory = std::filesystem::path(g_get_user_config_dir()) / App::k_ApplicationId / "scanners";

    if (!std::filesystem::exists(prefDirectory))
    {
        std::filesystem::create_directories(prefDirectory);
    }

    auto indexFilePath = prefDirectory / "index.json";
    std::filesystem::file_time_type indexModifiedTime;
    nlohmann::json index;

    if (!std::filesystem::exists(indexFilePath))
    {
        indexModifiedTime = std::filesystem::file_time_type::min();
    }
    else
    {
        indexModifiedTime = std::filesystem::last_write_time(indexFilePath);
        auto indexFile = std::ifstream(indexFilePath);
        index = nlohmann::json::parse(indexFile);
        indexFile.close();
    }

    if (index.empty() || !index.contains("contents") || !index["contents"].is_array())
    {
        index = nlohmann::json::object();
        index["contents"] = nlohmann::json::array();
    }

    UpdateIndexRecursively(baseDirectory, indexModifiedTime, index);
    UpdateIndexRecursively(prefDirectory, indexModifiedTime, index);

    auto newIndexFile = std::ofstream(indexFilePath);
    newIndexFile << index.dump(4);
    newIndexFile.close();
}

void ZooScan::OptionRewriter::LoadOptionDescriptionFile(const char *deviceVendor, const char *deviceModel)
{
    m_OptionInfos.clear();

    auto baseDirectory = std::filesystem::current_path() / "../scanners";
    auto prefDirectory = std::filesystem::path(g_get_user_config_dir()) / App::k_ApplicationId / "scanners";
    auto indexFilePath = prefDirectory / "index.json";

    nlohmann::json index;
    if (std::filesystem::exists(indexFilePath))
    {
        std::ifstream indexFile(indexFilePath);
        index = nlohmann::json::parse(indexFile);
        indexFile.close();
    }

    if (index.empty() || !index.contains("contents") || !index["contents"].is_array())
    {
        return;
    }

    auto optionFilePath = std::filesystem::path();
    for (auto vendor: index["contents"])
    {
        if (vendor["vendor"] == deviceVendor)
        {
            for (auto model: vendor["models"])
            {
                if (model["name"] == deviceModel)
                {
                    optionFilePath = baseDirectory / model["filepath"];
                    break;
                }
            }
        }

        if (!optionFilePath.empty())
        {
            break;
        }
    }

    if (std::filesystem::exists(optionFilePath))
    {
        std::ifstream optionFile(optionFilePath);
        auto optionFileData = nlohmann::json::parse(optionFile);
        optionFile.close();

        if (optionFileData.is_null() || !optionFileData.contains("options") || !optionFileData["options"].is_array())
        {
            return;
        }

        for (auto option: optionFileData["options"])
        {
            auto id = option["id"].get<uint32_t>();
            m_OptionInfos[id] = option.get<OptionInfos>();
        }
    }
}
