#pragma once

#include <sane/sane.h>

#include "Rect.hpp"
#include "ZooLib/ChangesetBase.hpp"
#include "ZooLib/ChangesetManager.hpp"
#include "ZooLib/StateComponent.hpp"


namespace ZooScan
{
    class PreviewStateChangeset : public ZooLib::ChangesetBase
    {
    public:
        enum class TypeFlag
        {
            None = 0,
            PanOffset = 1,
            ZoomFactor = 2,
            ScanArea = 4,
            Image = 8,
        };

    private:
        std::underlying_type_t<TypeFlag> m_ChangeType{};

    public:
        explicit PreviewStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        void Clear()
        {
            m_ChangeType = static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
        }

        void Set(TypeFlag typeFlag)
        {
            m_ChangeType |= static_cast<std::underlying_type_t<TypeFlag>>(typeFlag);
        }

        [[nodiscard]] bool HasAnyChange() const
        {
            return m_ChangeType != static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
        }

        [[nodiscard]] bool IsChanged(TypeFlag typeFlag) const
        {
            return (m_ChangeType & static_cast<std::underlying_type_t<TypeFlag>>(typeFlag)) != 0;
        }

        void Aggregate(const PreviewStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ChangeType |= changeset.m_ChangeType;
        }
    };

    class PreviewState final : public ZooLib::StateComponent, public ZooLib::ChangesetManager<PreviewStateChangeset>
    {
    public:
        static constexpr const char *k_ZoomValueStrings[] = {"-",    "12.5%", "25%",  "33%",  "50%",   "66%",
                                                             "100%", "200%",  "400%", "800%", "1600%", nullptr};
        static constexpr double k_ZoomValues[] = {0., .125, .25, .33, .5, .66, 1.0, 2.0, 4.0, 8.0, 16.0};

    private:
        Point<double> m_PanOffset{};
        double m_ZoomFactor{};

        Rect<double> m_ScanArea{};

        uint64_t m_ImageSize{}; // size, in bytes, of the image
        SANE_Byte *m_Image{}; // buffer to hold the scanned data
        uint64_t m_Offset{}; // offset in the image buffer where the next write will be done
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
            if (m_Image != nullptr)
            {
                free(m_Image);
            }
        }

        [[nodiscard]] Point<double> GetPreviewPanOffset() const
        {
            return m_PanOffset;
        }

        [[nodiscard]] double GetPreviewZoomFactor() const
        {
            return m_ZoomFactor;
        }

        [[nodiscard]] const Rect<double> &GetScanArea() const
        {
            return m_ScanArea;
        }

        [[nodiscard]] int GetScannedPixelsPerLine() const
        {
            return m_PixelsPerLine;
        }

        [[nodiscard]] int GetScannedImageHeight() const
        {
            return m_ImageHeight;
        }

        [[nodiscard]] const SANE_Byte *GetScannedImage() const
        {
            return m_Image;
        }

        [[nodiscard]] int GetScannedBytesPerLine() const
        {
            return m_BytesPerLine;
        }

        static double ClampToNearestZoomFactor(double zoomFactor)
        {
            auto nearestIndex = 1UZ;
            double distance = std::abs(zoomFactor - k_ZoomValues[1]);
            for (auto i = 2UZ; i < std::size(k_ZoomValues); ++i)
            {
                if (std::abs(zoomFactor - k_ZoomValues[i]) < distance)
                {
                    nearestIndex = i;
                    distance = std::abs(zoomFactor - k_ZoomValues[i]);
                }
            }

            return k_ZoomValues[nearestIndex];
        }

        static double FloorZoomFactor(double zoomFactor)
        {
            auto nearestIndex = 1UZ;
            double distance = zoomFactor - k_ZoomValues[1];
            for (auto i = 2UZ; i < std::size(k_ZoomValues); ++i)
            {
                auto d = zoomFactor - k_ZoomValues[i];
                if (d >= 0 && d < distance)
                {
                    nearestIndex = i;
                    distance = d;
                }
                else if (d < 0)
                {
                    break;
                }
            }

            return k_ZoomValues[nearestIndex];
        }

        class Updater final : public StateComponent::Updater<PreviewState>
        {
        public:
            explicit Updater(PreviewState *state)
                : StateComponent::Updater<PreviewState>(state)
            {
            }

            void SetPanOffset(Point<double> panOffset) const
            {
                if (m_StateComponent->m_PanOffset == panOffset)
                {
                    return;
                }

                m_StateComponent->m_PanOffset = panOffset;
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::PanOffset);
            }

            void SetZoomFactor(double zoomFactor) const
            {
                zoomFactor = ClampToNearestZoomFactor(zoomFactor);

                if (m_StateComponent->m_ZoomFactor == zoomFactor)
                {
                    return;
                }

                m_StateComponent->m_ZoomFactor = zoomFactor;
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::ZoomFactor);
            }

            void UpdatePreviewRectangle(const Rect<double> &scanArea) const
            {
                if (m_StateComponent->m_ScanArea == scanArea)
                {
                    return;
                }

                m_StateComponent->m_ScanArea = scanArea;
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::ScanArea);
            }

            void PrepareForScan(int pixelsPerLine, int bytesPerLine, int imageHeight) const
            {
                m_StateComponent->m_PixelsPerLine = pixelsPerLine;
                m_StateComponent->m_BytesPerLine = bytesPerLine;
                m_StateComponent->m_ImageHeight = imageHeight;

                const uint64_t requestedSize = bytesPerLine * imageHeight;
                if (m_StateComponent->m_Image != nullptr && m_StateComponent->m_ImageSize != requestedSize)
                {
                    free(m_StateComponent->m_Image);
                    m_StateComponent->m_Image = nullptr;
                }

                if (m_StateComponent->m_Image == nullptr)
                {
                    m_StateComponent->m_ImageSize = requestedSize;
                    m_StateComponent->m_Image = static_cast<SANE_Byte *>(calloc(m_StateComponent->m_ImageSize, 1));

                    auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                    changeset->Set(PreviewStateChangeset::TypeFlag::Image);
                }

                m_StateComponent->m_Offset = 0;
            }

            void GetReadBuffer(SANE_Byte *&buffer, int &maxLength) const
            {
                buffer = m_StateComponent->m_Image + m_StateComponent->m_Offset;
                maxLength = static_cast<int>(
                        std::min(1024UL * 1024UL, m_StateComponent->m_ImageSize - m_StateComponent->m_Offset));
            }

            void CommitReadBuffer(int readLength) const
            {
                m_StateComponent->m_Offset += readLength;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::Image);
            }

            void ResetReadBuffer() const
            {
                m_StateComponent->m_Offset = 0;
            }
        };
    };
}
