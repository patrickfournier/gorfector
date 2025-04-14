#pragma once

#include <tiff.h>

namespace ZooScan
{
    class TiffWriterState : public ZooLib::StateComponent
    {
    public:
        enum class Compression
        {
            None,
            LZW,
            JPEG,
            Deflate,
            Packbits
        };

        static std::vector<const char *> GetCompressionAlgorithmNames()
        {
            return {"None", "LZW", "JPEG", "Deflate", "Packbits", nullptr};
        }

    private:
        Compression m_Compression{};
        int m_DeflateCompressionLevel{};
        int m_JpegQuality{};

    public:
        explicit TiffWriterState(ZooLib::State *state)
            : StateComponent(state)
            , m_Compression(Compression::Deflate)
            , m_DeflateCompressionLevel(1)
            , m_JpegQuality(75)
        {
        }

        [[nodiscard]] Compression GetCompression() const
        {
            return m_Compression;
        }

        [[nodiscard]] int GetCompressionIndex() const
        {
            return static_cast<int>(m_Compression);
        }

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
                    return COMPRESSION_DEFLATE;
                case Compression::Packbits:
                    return COMPRESSION_PACKBITS;
            }

            return COMPRESSION_NONE;
        }

        [[nodiscard]] int GetDeflateCompressionLevel() const
        {
            return m_DeflateCompressionLevel;
        }

        [[nodiscard]] int GetJpegQuality() const
        {
            return m_JpegQuality;
        }

        class Updater final : public StateComponent::Updater<TiffWriterState>
        {
        public:
            explicit Updater(TiffWriterState *state)
                : StateComponent::Updater<TiffWriterState>(state)
            {
            }

            void SetCompression(Compression compression)
            {
                m_StateComponent->m_Compression = compression;
            }

            void SetDeflateCompressionLevel(int compressionLevel)
            {
                m_StateComponent->m_DeflateCompressionLevel = compressionLevel;
            }

            void SetJpegQuality(int quality)
            {
                m_StateComponent->m_JpegQuality = quality;
            }
        };
    };
}
