#pragma once

#include <algorithm>
#include <filesystem>
#include <sane/sane.h>
#include <string>
#include <utility>
#include <vector>

#include "ZooLib/State.hpp"

namespace Gorfector
{
    class App;
    class DeviceOptionsState;

    /**
     * \class FileWriter
     * \brief Base class for handling file writing operations in the application.
     *
     * This class provides an interface for managing file writing, including registering
     * file writers, handling file extensions, and managing file creation and writing errors.
     */
    class FileWriter
    {
    public:
        /**
         * \enum Error
         * \brief Enumeration of possible errors during file writing operations.
         */
        enum class Error
        {
            None, /**< No error occurred. */
            CannotOpenFile, /**< The file could not be opened. */
            ImageTooLarge, /**< The image size exceeds the allowed limit. */
            UnknownError /**< An unknown error occurred. */
        };

    private:
        /**
         * \brief Static list of registered file formats.
         */
        static std::vector<FileWriter *> s_Writers;

        /**
         * \brief Name of the application using the file writer.
         */
        std::string m_ApplicationName;

    protected:
        /**
         * \brief Retrieves the application name.
         * \return A constant reference to the application name string.
         */
        [[nodiscard]] const std::string &GetApplicationName() const
        {
            return m_ApplicationName;
        }

    public:
        /**
         * \brief Registers a new file writer.
         * \tparam TFileWriter The file writer class to register.
         * \param state Pointer to the application state.
         * \param applicationName Name of the application.
         */
        template<typename TFileWriter>
        static void Register(ZooLib::State *state, const std::string &applicationName)
        {
            static_assert(std::derived_from<TFileWriter, FileWriter>);

            if (std::ranges::any_of(s_Writers, [](const FileWriter *writer) {
                    return dynamic_cast<const TFileWriter *>(writer) != nullptr;
                }))
            {
                return;
            }

            auto f = new TFileWriter(state, applicationName);
            state->LoadFromPreferencesFile(f->GetStateComponent());
            s_Writers.push_back(f);
        }

        /**
         * \brief Clears all registered file writers.
         */
        static void Clear()
        {
            for (auto &f: s_Writers)
                delete f;

            s_Writers.clear();
        }

        /**
         * \brief Retrieves the list of registered file writers.
         * \return A constant reference to the list of file writers.
         */
        [[nodiscard]] static const std::vector<FileWriter *> &GetWriters()
        {
            return s_Writers;
        }

        /**
         * \brief Retrieves a file writer by its type.
         * \tparam TFileWriter The type of the file writer to retrieve.
         * \return A pointer to the file writer of the specified type, or nullptr if not found.
         */
        template<typename TFileWriter>
        [[nodiscard]] static TFileWriter *GetFormatByType()
        {
            static_assert(std::derived_from<TFileWriter, FileWriter>);

            for (auto writer: s_Writers)
            {
                auto fileWriter = dynamic_cast<TFileWriter *>(writer);
                if (fileWriter != nullptr)
                {
                    return fileWriter;
                }
            }

            return nullptr;
        }

        /**
         * \brief Retrieves a file writer based on the file path extension.
         * \param path The file path to check.
         * \return A pointer to the matching file writer, or nullptr if no match is found.
         */
        [[nodiscard]] static FileWriter *GetFileWriterForPath(const std::filesystem::path &path)
        {
            for (auto writer: s_Writers)
            {
                auto pathExtension = path.extension();
                for (const auto &ext: writer->GetExtensions())
                {
                    if (pathExtension.string() == ext)
                        return writer;
                }
            }

            return nullptr;
        }

        /**
         * \brief Constructor for the FileWriter class.
         * \param applicationName The name of the application using the file writer.
         */
        explicit FileWriter(std::string applicationName)
            : m_ApplicationName(std::move(applicationName))
        {
        }

        /**
         * \brief Virtual destructor for the FileWriter class.
         */
        virtual ~FileWriter() = default;

        /**
         * \brief Retrieves the name of the file writer.
         * \return A constant reference to the name string.
         */
        [[nodiscard]] virtual const std::string &GetName() const = 0;

        /**
         * \brief Retrieves the supported file extensions for the file writer.
         * \return A vector of strings containing the supported extensions.
         */
        [[nodiscard]] virtual std::vector<std::string> GetExtensions() const = 0;

        /**
         * \brief Creates a new file for writing.
         * \param path The file path to create.
         * \param deviceOptions Pointer to the device options state.
         * \param parameters SANE parameters for the file.
         * \return An error code indicating the result of the operation.
         */
        virtual Error CreateFile(
                std::filesystem::path &path, const DeviceOptionsState *deviceOptions,
                const SANE_Parameters &parameters) = 0;

        /**
         * \brief Appends bytes to the file.
         * \param bytes Pointer to the byte data.
         * \param numberOfLines Number of lines to append.
         * \param parameters SANE parameters for the file.
         * \return The number of bytes appended.
         */
        virtual uint32_t AppendBytes(SANE_Byte *bytes, int numberOfLines, const SANE_Parameters &parameters) = 0;

        /**
         * \brief Closes the file after writing.
         */
        virtual void CloseFile() = 0;

        /**
         * \brief Cancels the file writing operation.
         */
        virtual void CancelFile() = 0;

        /**
         * \brief Retrieves a human-readable error message for a given error code.
         * \param error The error code.
         * \return A string describing the error.
         */
        virtual std::string GetError(Error error)
        {
            switch (error)
            {
                case Error::None:
                    return "No error";
                case Error::CannotOpenFile:
                    return "Cannot open file";
                case Error::ImageTooLarge:
                    return "Image too large";
                case Error::UnknownError:
                default:
                    return "Unknown error";
            }
        }
    };
}
