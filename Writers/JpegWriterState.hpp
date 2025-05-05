#pragma once

#include "ZooLib/StateComponent.hpp"

namespace Gorfector
{
    /**
     * \class JpegWriterState
     * \brief Represents the state of a JPEG writer, including quality settings.
     *
     * This class is responsible for managing the state of a JPEG writer, such as the quality of the JPEG output.
     * It provides serialization and deserialization capabilities using JSON and allows updates to the state
     * through its nested `Updater` class.
     */
    class JpegWriterState : public ZooLib::StateComponent
    {
    public:
        /**
         * \brief Key used for storing the quality value in JSON.
         */
        static constexpr const char *k_QualityKey = "Quality";

    private:
        int m_Quality; ///< The quality of the JPEG output (default is 75).

        friend void to_json(nlohmann::json &j, const JpegWriterState &p);
        friend void from_json(const nlohmann::json &j, JpegWriterState &p);

    public:
        /**
         * \brief Constructs a `JpegWriterState` with a default quality of 75.
         * \param state The parent state object.
         */
        explicit JpegWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_Quality(75)
        {
        }

        /**
         * \brief Destructor that saves the state to a file.
         */
        ~JpegWriterState() override
        {
            m_State->SaveToFile(this);
        }

        /**
         * \brief Gets the serialization key for this state.
         * \return A string representing the serialization key.
         */
        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "JpegWriterState";
        }

        /**
         * \brief Gets the current JPEG quality setting.
         * \return The quality value.
         */
        [[nodiscard]] int GetQuality() const
        {
            return m_Quality;
        }

        /**
         * \class Updater
         * \brief A helper class for updating the `JpegWriterState`.
         *
         * This nested class provides methods to update the state, including loading from JSON
         * and setting the quality value.
         */
        class Updater final : public StateComponent::Updater<JpegWriterState>
        {
        public:
            /**
             * \brief Constructs an `Updater` for the given `JpegWriterState`.
             * \param state The `JpegWriterState` instance to update.
             */
            explicit Updater(JpegWriterState *state)
                : StateComponent::Updater<JpegWriterState>(state)
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
             * \brief Sets the JPEG quality value.
             * \param quality The new quality value.
             */
            void SetQuality(int quality) const
            {
                m_StateComponent->m_Quality = quality;
            }
        };
    };

    /**
     * \brief Serializes the `JpegWriterState` to JSON.
     * \param j The JSON object to populate.
     * \param p The `JpegWriterState` instance to serialize.
     */
    inline void to_json(nlohmann::json &j, const JpegWriterState &p)
    {
        j = nlohmann::json{{JpegWriterState::k_QualityKey, p.m_Quality}};
    }

    /**
     * \brief Deserializes the `JpegWriterState` from JSON.
     * \param j The JSON object to read from.
     * \param p The `JpegWriterState` instance to populate.
     */
    inline void from_json(const nlohmann::json &j, JpegWriterState &p)
    {
        j.at(JpegWriterState::k_QualityKey).get_to(p.m_Quality);
    }
}
