#include "PathUtils.hpp"

#include <filesystem>
#include <format>
#include <regex>

#include "config.h"

void ZooLib::IncrementPath(std::filesystem::path &path)
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

static std::filesystem::path s_RelocationPath;

void InitRelocatePath()
{
    auto exec_path = std::filesystem::canonical("/proc/self/exe");
    auto exec_filename = exec_path.filename();
    auto install_bin = std::filesystem::path(BIN_DIR) / exec_filename;

    if (exec_path.string().ends_with(install_bin.string()))
    {
        auto relocation_path = exec_path.string();
        relocation_path = relocation_path.substr(0, relocation_path.length() - install_bin.string().length());
        s_RelocationPath = std::filesystem::path(relocation_path);
    }
    else
    {
        s_RelocationPath = "/";
    }
}

std::filesystem::path ZooLib::RelocatePath(const std::filesystem::path &path)
{
    if (s_RelocationPath.empty())
    {
        InitRelocatePath();
    }

    auto absPath = std::filesystem::absolute(path);
    return s_RelocationPath / std::filesystem::path(absPath).relative_path();
}

std::filesystem::path ZooLib::UnrelocatePath(const std::filesystem::path &path)
{
    if (s_RelocationPath.empty())
    {
        InitRelocatePath();
    }

    auto canonPath = std::filesystem::weakly_canonical(path);

    auto pathStr = canonPath.string();
    auto relocationStr = s_RelocationPath.string();
    if (pathStr.starts_with(relocationStr))
    {
        return "/" / std::filesystem::path(pathStr.substr(relocationStr.length()));
    }

    return canonPath;
}
