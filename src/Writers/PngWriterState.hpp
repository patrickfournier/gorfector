#pragma once

#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    /**
     * \brief Represents the state of the PNG writer component.
     *
     * This class manages the state of the PNG writer, including its compression level.
     * It provides serialization and deserialization functionality using JSON.
     * The state is automatically saved to a file upon destruction.
     */
    class PngWriterState : public ZooLib::StateComponent
    {
    public:
        /**
         * \brief Key used for the compression level in JSON serialization.
         */
        static constexpr const char *k_CompressionLevelKey = "CompressionLevel";

    private:
        /**
         * \brief The compression level for the PNG writer.
         *
         * This value determines the level of compression applied when writing PNG files.
         * The default value is 1.
         */
        int m_CompressionLevel{};

        friend void to_json(nlohmann::json &j, const PngWriterState &p);
        friend void from_json(const nlohmann::json &j, PngWriterState &p);

    public:
        /**
         * \brief Constructs a PngWriterState instance.
         * \param state The parent state object.
         */
        explicit PngWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_CompressionLevel(1)
        {
        }

        /**
         * \brief Destructor that saves the state to a file.
         */
        ~PngWriterState() override
        {
            m_State->SaveToFile(this);
        }

        /**
         * \brief Gets the serialization key for this state.
         * \return A string representing the serialization key.
         */
        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "PngWriterState";
        }

        /**
         * \brief Gets the current compression level.
         * \return The compression level as an integer.
         */
        [[nodiscard]] int GetCompressionLevel() const
        {
            return m_CompressionLevel;
        }

        /**
         * \brief Provides an updater for modifying the state.
         *
         * The updater allows controlled modifications to the state, such as setting the compression level
         * or loading data from JSON.
         */
        class Updater final : public StateComponent::Updater<PngWriterState>
        {
        public:
            /**
             * \brief Constructs an updater for the given state.
             * \param state The PngWriterState instance to update.
             */
            explicit Updater(PngWriterState *state)
                : StateComponent::Updater<PngWriterState>(state)
            {
            }

            /**
             * \brief Loads the state from a JSON object.
             * \param json The JSON object containing the state data.
             */
            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            /**
             * \brief Sets the compression level.
             * \param compressionLevel The new compression level to set.
             */
            void SetCompressionLevel(int compressionLevel) const
            {
                m_StateComponent->m_CompressionLevel = compressionLevel;
            }
        };
    };

    /**
     * \brief Serializes the PngWriterState to JSON.
     * \param j The JSON object to populate.
     * \param p The PngWriterState instance to serialize.
     */
    inline void to_json(nlohmann::json &j, const PngWriterState &p)
    {
        j = nlohmann::json{{PngWriterState::k_CompressionLevelKey, p.m_CompressionLevel}};
    }

    /**
     * \brief Deserializes the PngWriterState from JSON.
     * \param j The JSON object containing the state data.
     * \param p The PngWriterState instance to populate.
     */
    inline void from_json(const nlohmann::json &j, PngWriterState &p)
    {
        j.at(PngWriterState::k_CompressionLevelKey).get_to(p.m_CompressionLevel);
    }
}
