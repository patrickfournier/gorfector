#pragma once

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <sane/sane.h>
#include <string>
#include <vector>

namespace ZooLib
{
    class State;
}

namespace ZooScan
{
    class App;
    class DeviceOptionsState;

    class FileWriter
    {
    public:
        enum class Error
        {
            None,
            CannotOpenFile
        };

    private:
        static std::vector<FileWriter *> s_Formats;

    public:
        template<typename TFileFormat>
        static void Register(ZooLib::State *state)
        {
            static_assert(std::derived_from<TFileFormat, FileWriter>);

            if (std::ranges::any_of(s_Formats, [](const FileWriter *format) {
                    return dynamic_cast<const TFileFormat *>(format) != nullptr;
                }))
            {
                return;
            }

            auto f = new TFileFormat(state);
            s_Formats.push_back(f);
        }

        static void Clear()
        {
            for (auto &f: s_Formats)
                delete f;

            s_Formats.clear();
        }

        [[nodiscard]] static const std::vector<FileWriter *> &GetFormats()
        {
            return s_Formats;
        }

        [[nodiscard]] static FileWriter *GetFormatByName(const std::string &name)
        {
            for (auto format: s_Formats)
            {
                if (format->GetName() == name)
                    return format;
            }

            return nullptr;
        }

        [[nodiscard]] static FileWriter *GetFormatForPath(const std::filesystem::path &path)
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

        virtual ~FileWriter() = default;

        [[nodiscard]] virtual const std::string &GetName() const = 0;
        [[nodiscard]] virtual std::vector<std::string> GetExtensions() const = 0;

        [[nodiscard]] bool IsKnownExtension(const std::filesystem::path &path) const
        {
            auto pathExtension = path.extension();

            return std::ranges::any_of(GetExtensions(), [pathExtension](const std::filesystem::path &ext) {
                return pathExtension.string() == ext;
            });
        }

        void AddExtension(std::filesystem::path &path) const
        {
            path = path.string() + GetExtensions()[0];
        }

        virtual Error CreateFile(
                const App &app, std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters, SANE_Byte *image) = 0;
        virtual int32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, int pixelsPerLine, int bytesPerLine) = 0;
        virtual void CloseFile() = 0;

        virtual std::string GetError(Error error)
        {
            switch (error)
            {
                case Error::None:
                    return "No error";
                case Error::CannotOpenFile:
                    return "Cannot open file";
                default:
                    return "Unknown error";
            }
        }
    };
}
