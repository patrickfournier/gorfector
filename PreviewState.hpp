#pragma once

#include <sane/sane.h>

#include "Rect.hpp"
#include "ZooLib/StateComponent.hpp"


namespace ZooScan
{
    class PreviewState final : public ZooLib::StateComponent
    {
        Rect<double> m_ScanArea{};

        uint64_t m_FullImageSize{};
        SANE_Byte *m_FullImage{};
        uint64_t m_Offset{};
        int m_PixelsPerLine{};
        int m_BytesPerLine{};
        int m_ImageHeight{};

    public:
        explicit PreviewState(ZooLib::State *state)
            : StateComponent(state)
        {
        }

        ~PreviewState() override
        {
        }

        [[nodiscard]] const Rect<double> &GetScanArea() const
        {
            return m_ScanArea;
        }

        [[nodiscard]] int GetPixelsPerLine() const
        {
            return m_PixelsPerLine;
        }

        [[nodiscard]] int GetImageHeight() const
        {
            return m_ImageHeight;
        }

        [[nodiscard]] const SANE_Byte *GetFullImage() const
        {
            return m_FullImage;
        }

        [[nodiscard]] int GetBytesPerLine() const
        {
            return m_BytesPerLine;
        }

        class Updater final : public StateComponent::Updater<PreviewState>
        {
        public:
            explicit Updater(PreviewState *state)
                : StateComponent::Updater<PreviewState>(state)
            {
            }

            void UpdatePreviewRectangle(const Rect<double> &scanArea) const
            {
                m_StateComponent->m_ScanArea = scanArea;
            }

            void PrepareForScan(int pixelsPerLine, int bytesPerLine, int imageHeight) const
            {
                m_StateComponent->m_PixelsPerLine = pixelsPerLine;
                m_StateComponent->m_BytesPerLine = bytesPerLine;
                m_StateComponent->m_ImageHeight = imageHeight;

                m_StateComponent->m_FullImageSize = bytesPerLine * imageHeight;
                m_StateComponent->m_FullImage = static_cast<SANE_Byte *>(calloc(m_StateComponent->m_FullImageSize, 1));

                m_StateComponent->m_Offset = 0;
            }

            void GetReadBuffer(SANE_Byte *&buffer, int &maxLength) const
            {
                buffer = m_StateComponent->m_FullImage + m_StateComponent->m_Offset;
                maxLength = static_cast<int>(
                        std::min(1024UL * 1024UL, m_StateComponent->m_FullImageSize - m_StateComponent->m_Offset));
            }

            void CommitReadBuffer(int readLength) const
            {
                m_StateComponent->m_Offset += readLength;
            }

            void ResetReadBuffer() const
            {
                free(m_StateComponent->m_FullImage);
                m_StateComponent->m_FullImage = nullptr;
                m_StateComponent->m_FullImageSize = 0;
                m_StateComponent->m_Offset = 0;
            }
        };
    };
}
