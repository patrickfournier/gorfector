#pragma once

#include "ZooLib/Gettext.hpp"
#include "ZooLib/StateComponent.hpp"

namespace ZooScan
{

    class OutputOptionsState : public ZooLib::StateComponent
    {
    public:
        enum class OutputDestination
        {
            e_File,
            e_Printer,
            e_Email,
        };
        const char *const k_OutputDestinationList[4]{_("File"), _("Printer"), _("Email"), nullptr};

        enum class FileExistsAction
        {
            e_IncrementCounter,
            e_Overwrite,
            e_Cancel
        };
        const char *const k_FileExistsActionList[4]{_("Increase Counter"), _("Overwrite"), _("Cancel Scan"), nullptr};

    private:
        OutputDestination m_OutputDestination{};
        std::filesystem::path m_OutputDirectory{};
        bool m_CreateMissingDirectories{true};
        std::string m_OutputFileName{};
        FileExistsAction m_FileExistsAction{};

    public:
        explicit OutputOptionsState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        [[nodiscard]] OutputDestination GetOutputDestination() const
        {
            return m_OutputDestination;
        }
        [[nodiscard]] const std::filesystem::path &GetOutputDirectory() const
        {
            return m_OutputDirectory;
        }
        [[nodiscard]] bool GetCreateMissingDirectories() const
        {
            return m_CreateMissingDirectories;
        }
        [[nodiscard]] const std::string &GetOutputFileName() const
        {
            return m_OutputFileName;
        }
        [[nodiscard]] FileExistsAction GetFileExistsAction() const
        {
            return m_FileExistsAction;
        }

        class Updater : public StateComponent::Updater<OutputOptionsState>
        {
        public:
            explicit Updater(OutputOptionsState *state)
                : StateComponent::Updater<OutputOptionsState>(state)
            {
            }

            void SetOutputDestination(OutputDestination destination)
            {
                m_StateComponent->m_OutputDestination = destination;
            }

            void SetOutputDirectory(const std::filesystem::path &directory)
            {
                m_StateComponent->m_OutputDirectory = directory;
            }

            void SetCreateMissingDirectories(bool create)
            {
                m_StateComponent->m_CreateMissingDirectories = create;
            }

            void SetOutputFileName(const std::string &filename)
            {
                m_StateComponent->m_OutputFileName = filename;
            }

            void SetFileExistsAction(FileExistsAction action)
            {
                m_StateComponent->m_FileExistsAction = action;
            }
        };
    };

}
