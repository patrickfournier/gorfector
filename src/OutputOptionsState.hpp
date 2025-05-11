#pragma once

#include "ZooLib/Gettext.hpp"
#include "ZooLib/State.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    /**
     * \brief Manages the output options, including destination, directory, and file handling.
     *
     * This class encapsulates the configuration for output options, such as the destination type,
     * output directory, file name, and actions to take when a file already exists. It provides
     * serialization and deserialization support using JSON.
     */
    class OutputOptionsState : public ZooLib::StateComponent
    {
    public:
        /**
         * \brief Keys used for serialization and deserialization of the state.
         */
        static constexpr const char *k_OutputDestinationKey = "OutputDestination"; ///< Key for output destination.
        static constexpr const char *k_OutputDirectoryKey = "OutputDirectory"; ///< Key for output directory.
        static constexpr const char *k_CreateMissingDirectoriesKey =
                "CreateMissingDirectories"; ///< Key for directory creation flag.
        static constexpr const char *k_OutputFileNameKey = "OutputFileName"; ///< Key for output file name.
        static constexpr const char *k_FileExistsActionKey = "FileExistsAction"; ///< Key for file exists action.

        /**
         * \brief Enum representing the possible output destinations.
         */
        enum class OutputDestination
        {
            e_File, ///< Output to a file.
            e_Printer, ///< Output to a printer.
            e_Email, ///< Output to an email.
        };
        const char *const k_OutputDestinationList[4]{
                _("File"), _("Printer"), _("Email"), nullptr}; ///< List of output destination names.

        /**
         * \brief Enum representing actions to take when a file already exists.
         */
        enum class FileExistsAction
        {
            e_IncrementCounter, ///< Increment the file name counter.
            e_Overwrite, ///< Overwrite the existing file.
            e_Cancel ///< Cancel the operation.
        };
        const char *const k_FileExistsActionList[4]{
                _("Increase Counter"), _("Overwrite"), _("Cancel Scan"),
                nullptr}; ///< List of file exists action names.

    private:
        OutputDestination m_OutputDestination{}; ///< The selected output destination.
        std::filesystem::path m_OutputDirectory{}; ///< The directory where output will be saved.
        bool m_CreateMissingDirectories{true}; ///< Whether to create missing directories.
        std::string m_OutputFileName{}; ///< The name of the output file.
        FileExistsAction m_FileExistsAction{}; ///< The action to take if the file already exists.

        friend void to_json(nlohmann::json &j, const OutputOptionsState &p);
        friend void from_json(const nlohmann::json &j, OutputOptionsState &p);

    public:
        /**
         * \brief Constructs the state with a reference to the parent state.
         *
         * \param state The parent state.
         */
        explicit OutputOptionsState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        /**
         * \brief Destructor that saves the state to a file.
         */
        ~OutputOptionsState() override
        {
            m_State->SaveToFile(this);
        }

        /**
         * \brief Gets the serialization key for this state.
         *
         * \return The serialization key.
         */
        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "OutputOptionsState";
        }

        /**
         * \brief Gets the output destination.
         *
         * \return The output destination.
         */
        [[nodiscard]] OutputDestination GetOutputDestination() const
        {
            return m_OutputDestination;
        }

        /**
         * \brief Gets the output directory.
         *
         * \return The output directory.
         */
        [[nodiscard]] const std::filesystem::path &GetOutputDirectory() const
        {
            return m_OutputDirectory;
        }

        /**
         * \brief Checks if missing directories should be created.
         *
         * \return True if missing directories should be created, false otherwise.
         */
        [[nodiscard]] bool GetCreateMissingDirectories() const
        {
            return m_CreateMissingDirectories;
        }

        /**
         * \brief Gets the output file name.
         *
         * \return The output file name.
         */
        [[nodiscard]] const std::string &GetOutputFileName() const
        {
            return m_OutputFileName;
        }

        /**
         * \brief Gets the action to take if the file already exists.
         *
         * \return The file exists action.
         */
        [[nodiscard]] FileExistsAction GetFileExistsAction() const
        {
            return m_FileExistsAction;
        }

        /**
         * \brief Updater class for modifying the state.
         */
        class Updater : public StateComponent::Updater<OutputOptionsState>
        {
        public:
            /**
             * \brief Constructs the updater with a reference to the state.
             *
             * \param state The state to update.
             */
            explicit Updater(OutputOptionsState *state)
                : StateComponent::Updater<OutputOptionsState>(state)
            {
            }

            /**
             * \brief Loads the state from a JSON object.
             *
             * \param json The JSON object to load from.
             */
            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            /**
             * \brief Applies settings from a JSON object.
             *
             * \param json The JSON object containing the settings.
             */
            void ApplySettings(const nlohmann::json &json)
            {
                LoadFromJson(json);
            }

            /**
             * \brief Sets the output destination.
             *
             * \param destination The new output destination.
             */
            void SetOutputDestination(OutputDestination destination)
            {
                m_StateComponent->m_OutputDestination = destination;
            }

            /**
             * \brief Sets the output directory.
             *
             * \param directory The new output directory.
             */
            void SetOutputDirectory(const std::filesystem::path &directory)
            {
                m_StateComponent->m_OutputDirectory = directory;
            }

            /**
             * \brief Sets whether to create missing directories.
             *
             * \param create True to create missing directories, false otherwise.
             */
            void SetCreateMissingDirectories(bool create)
            {
                m_StateComponent->m_CreateMissingDirectories = create;
            }

            /**
             * \brief Sets the output file name.
             *
             * \param filename The new output file name.
             */
            void SetOutputFileName(const std::string &filename)
            {
                m_StateComponent->m_OutputFileName = filename;
            }

            /**
             * \brief Sets the action to take if the file already exists.
             *
             * \param action The new file exists action.
             */
            void SetFileExistsAction(FileExistsAction action)
            {
                m_StateComponent->m_FileExistsAction = action;
            }
        };
    };

    /**
     * \brief Serializes the `OutputOptionsState` object to a JSON object.
     *
     * This function converts the `OutputOptionsState` object into a JSON representation
     * for serialization purposes. It includes all relevant state properties.
     *
     * \param j The JSON object to populate with the serialized data.
     * \param p The `OutputOptionsState` object to serialize.
     */
    inline void to_json(nlohmann::json &j, const OutputOptionsState &p)
    {
        auto outputDir = p.m_OutputDirectory.string();
        j = nlohmann::json{
                {OutputOptionsState::k_OutputDestinationKey, p.m_OutputDestination},
                {OutputOptionsState::k_OutputDirectoryKey, outputDir},
                {OutputOptionsState::k_CreateMissingDirectoriesKey, p.m_CreateMissingDirectories},
                {OutputOptionsState::k_OutputFileNameKey, p.m_OutputFileName},
                {OutputOptionsState::k_FileExistsActionKey, p.m_FileExistsAction}};
    }

    /**
     * \brief Deserializes a JSON object into an `OutputOptionsState` object.
     *
     * This function populates an `OutputOptionsState` object with data from a JSON
     * representation. It extracts all relevant state properties from the JSON object.
     *
     * \param j The JSON object containing the serialized data.
     * \param p The `OutputOptionsState` object to populate.
     */
    inline void from_json(const nlohmann::json &j, OutputOptionsState &p)
    {
        j.at(OutputOptionsState::k_OutputDestinationKey).get_to(p.m_OutputDestination);
        j.at(OutputOptionsState::k_OutputDirectoryKey).get_to(p.m_OutputDirectory);
        j.at(OutputOptionsState::k_CreateMissingDirectoriesKey).get_to(p.m_CreateMissingDirectories);
        j.at(OutputOptionsState::k_OutputFileNameKey).get_to(p.m_OutputFileName);
        j.at(OutputOptionsState::k_FileExistsActionKey).get_to(p.m_FileExistsAction);
    }
}
