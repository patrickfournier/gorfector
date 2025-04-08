#pragma once

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <sane/sane.h>
#include <string>
#include <vector>

namespace ZooScan
{
    class DeviceOptionsState;

    class FileFormat
    {
        static std::vector<const FileFormat *> s_Formats;

    public:
        template<typename TFileFormat>
        static void Register()
        {
            static_assert(std::derived_from<TFileFormat, FileFormat>);

            if (std::ranges::any_of(s_Formats, [](const FileFormat *format) {
                    return dynamic_cast<const TFileFormat *>(format) != nullptr;
                }))
            {
                return;
            }

            auto f = new TFileFormat();
            s_Formats.push_back(f);
        }

        static void Clear()
        {
            for (auto &f: s_Formats)
                delete f;

            s_Formats.clear();
        }

        [[nodiscard]] static const std::vector<const FileFormat *> &GetFormats()
        {
            return s_Formats;
        }

        [[nodiscard]] static const FileFormat *GetFormatByName(const std::string &name)
        {
            for (auto format: s_Formats)
            {
                if (format->GetName() == name)
                    return format;
            }

            return nullptr;
        }

        [[nodiscard]] static const FileFormat *GetFormatForPath(const std::filesystem::path &path)
        {
            for (auto format: s_Formats)
            {
                auto pathExtension = path.extension();
                for (const auto &ext: format->GetExtensions())
                {
                    if (pathExtension.string() == ext)
                        return format;
                }
            }

            return nullptr;
        }

        [[nodiscard]] static std::string DefaultFileName()
        {
            if (s_Formats.empty())
            {
                throw std::runtime_error("No output formats registered");
            }

            auto defaultFormat = s_Formats[0];
            auto defaultExtension = defaultFormat->GetExtensions()[0];

            return "scan" + defaultExtension;
        }

        virtual ~FileFormat() = default;

        [[nodiscard]] virtual const std::string &GetName() const = 0;
        [[nodiscard]] virtual std::vector<std::string> GetExtensions() const = 0;

        [[nodiscard]] bool IsKnownExtension(const std::filesystem::path &path) const
        {
            auto pathExtensions = path.extension();
            for (const auto &ext: GetExtensions())
            {
                if (pathExtensions.string() == ext)
                    return true;
            }
            return false;
        }

        void AddExtension(std::filesystem::path &path) const
        {
            path = path.string() + GetExtensions()[0];
        }

        virtual void
        Save(std::filesystem::path &path, const DeviceOptionsState *deviceOptions, const SANE_Parameters &parameters,
             SANE_Byte *image) const = 0;
    };
}
