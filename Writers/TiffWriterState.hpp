#pragma once

#include <tiff.h>

namespace Gorfector
{
    /**
     * @class TiffWriterState
     * @brief Manages the state for writing TIFF files, including compression settings.
     *
     * This class is responsible for handling the configuration and state of TIFF file writing.
     * It provides options for compression algorithms, compression levels, and JPEG quality.
     * The state is serialized to and from JSON for persistence.
     */
    class TiffWriterState : public ZooLib::StateComponent
    {
    public:
        /**
         * @brief Key for specifying the compression algorithm in JSON.
         */
        static constexpr const char *k_CompressionKey = "Compression";

        /**
         * @brief Key for specifying the compression level in JSON.
         */
        static constexpr const char *k_CompressionLevelKey = "CompressionLevel";

        /**
         * @brief Key for specifying the JPEG quality in JSON.
         */
        static constexpr const char *k_JpegQualityKey = "JpegQuality";

        /**
         * @enum Compression
         * @brief Enum representing supported compression algorithms.
         */
        enum class Compression
        {
            None, ///< No compression.
            LZW, ///< LZW compression.
            JPEG, ///< JPEG compression.
            Deflate, ///< Deflate compression.
            Packbits ///< Packbits compression.
        };

        /**
         * @brief Retrieves the names of supported compression algorithms.
         * @return A vector of strings representing the names of compression algorithms.
         */
        static std::vector<const char *> GetCompressionAlgorithmNames()
        {
            return {"None", "LZW", "JPEG", "Deflate", "Packbits", nullptr};
        }

    private:
        Compression m_Compression{}; ///< Current compression algorithm.
        int m_DeflateCompressionLevel{}; ///< Compression level for Deflate algorithm.
        int m_JpegQuality{}; ///< Quality level for JPEG compression.

        friend void to_json(nlohmann::json &j, const TiffWriterState &p);
        friend void from_json(const nlohmann::json &j, TiffWriterState &p);

    public:
        /**
         * @brief Constructs a TiffWriterState with default settings.
         * @param state Pointer to the parent ZooLib::State.
         */
        explicit TiffWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_Compression(Compression::Deflate)
            , m_DeflateCompressionLevel(1)
            , m_JpegQuality(75)
        {
        }

        /**
         * @brief Destructor that saves the state to a file.
         */
        ~TiffWriterState() override
        {
            m_State->SaveToFile(this);
        }

        /**
         * @brief Retrieves the serialization key for the state.
         * @return A string representing the serialization key.
         */
        [[nodiscard]] std::string GetSerializationKey() const override
        {
            return "TiffWriterState";
        }

        /**
         * @brief Retrieves the current compression algorithm.
         * @return The current compression algorithm as a `Compression` enum value.
         */
        [[nodiscard]] Compression GetCompression() const
        {
            return m_Compression;
        }

        /**
         * @brief Retrieves the index of the current compression algorithm.
         * @return An integer representing the index of the current compression algorithm.
         */
        [[nodiscard]] int GetCompressionIndex() const
        {
            return static_cast<int>(m_Compression);
        }

        /**
         * @brief Retrieves the TIFF-specific compression constant.
         * @return An integer representing the TIFF compression constant.
         */
        [[nodiscard]] int GetTiffCompression() const
        {
            switch (m_Compression)
            {
                case Compression::None:
                    return COMPRESSION_NONE;
                case Compression::LZW:
                    return COMPRESSION_LZW;
                case Compression::JPEG:
                    return COMPRESSION_JPEG;
                case Compression::Deflate:
                    return COMPRESSION_ADOBE_DEFLATE;
                case Compression::Packbits:
                    return COMPRESSION_PACKBITS;
            }

            return COMPRESSION_NONE;
        }

        /**
         * @brief Retrieves the compression level for the Deflate algorithm.
         * @return An integer representing the Deflate compression level.
         */
        [[nodiscard]] int GetDeflateCompressionLevel() const
        {
            return m_DeflateCompressionLevel;
        }

        /**
         * @brief Retrieves the quality level for JPEG compression.
         * @return An integer representing the JPEG quality level.
         */
        [[nodiscard]] int GetJpegQuality() const
        {
            return m_JpegQuality;
        }

        /**
         * @class Updater
         * @brief A helper class to update the state of `TiffWriterState`.
         *
         * This class provides methods to modify the state of `TiffWriterState` and
         * load its configuration from a JSON object.
         */
        class Updater final : public StateComponent::Updater<TiffWriterState>
        {
        public:
            /**
             * @brief Constructs an Updater for the given `TiffWriterState`.
             * @param state Pointer to the `TiffWriterState` to be updated.
             */
            explicit Updater(TiffWriterState *state)
                : StateComponent::Updater<TiffWriterState>(state)
            {
            }

            /**
             * @brief Loads the state from a JSON object.
             * @param json The JSON object containing the state configuration.
             */
            void LoadFromJson(const nlohmann::json &json) override
            {
                from_json(json, *m_StateComponent);
            }

            /**
             * @brief Sets the compression algorithm.
             * @param compression The compression algorithm to set.
             */
            void SetCompression(Compression compression) const
            {
                m_StateComponent->m_Compression = compression;
            }

            /**
             * @brief Sets the compression level for the Deflate algorithm.
             * @param compressionLevel The compression level to set.
             */
            void SetDeflateCompressionLevel(int compressionLevel) const
            {
                m_StateComponent->m_DeflateCompressionLevel = compressionLevel;
            }

            /**
             * @brief Sets the quality level for JPEG compression.
             * @param quality The JPEG quality level to set.
             */
            void SetJpegQuality(int quality) const
            {
                m_StateComponent->m_JpegQuality = quality;
            }
        };
    };

    /**
     * @brief Serializes a `TiffWriterState` object to a JSON object.
     * @param j The JSON object to populate.
     * @param p The `TiffWriterState` object to serialize.
     */
    inline void to_json(nlohmann::json &j, const TiffWriterState &p)
    {
        j = nlohmann::json{
                {TiffWriterState::k_CompressionKey, p.m_Compression},
                {TiffWriterState::k_CompressionLevelKey, p.m_DeflateCompressionLevel},
                {TiffWriterState::k_JpegQualityKey, p.m_JpegQuality}};
    }

    /**
     * @brief Deserializes a `TiffWriterState` object from a JSON object.
     * @param j The JSON object containing the state configuration.
     * @param p The `TiffWriterState` object to populate.
     */
    inline void from_json(const nlohmann::json &j, TiffWriterState &p)
    {
        j.at(TiffWriterState::k_CompressionKey).get_to(p.m_Compression);
        j.at(TiffWriterState::k_CompressionLevelKey).get_to(p.m_DeflateCompressionLevel);
        j.at(TiffWriterState::k_JpegQualityKey).get_to(p.m_JpegQuality);
    }
}
