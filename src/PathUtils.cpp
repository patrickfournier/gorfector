#include "PathUtils.hpp"

#include <format>
#include <regex>

void Gorfector::IncrementPath(std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
        return;

    auto extension = path.extension();
    auto filename = path.filename().replace_extension().string();
    auto directory = path.parent_path();

    int counter;
    std::string fileNameFormat;

    auto counterRegex = std::regex("(.+)([.\\-_])([0-9]+)$");
    std::smatch match;
    if (std::regex_match(filename, match, counterRegex) && match.size() == 4)
    {
        auto baseName = match[1].str();
        auto separator = match[2].str();
        counter = std::stoi(match[3].str());
        fileNameFormat =
                baseName + separator + "{:0" + std::to_string(match[3].str().length()) + "d}" + extension.string();
    }
    else
    {
        counter = 1;
        fileNameFormat = filename + "_{:02d}" + extension.string();
    }

    auto newFilePath = directory / std::vformat(fileNameFormat, std::make_format_args(counter));
    while (std::filesystem::exists(newFilePath))
    {
        counter++;
        newFilePath = directory / std::vformat(fileNameFormat, std::make_format_args(counter));
    }
    path = newFilePath;
}
