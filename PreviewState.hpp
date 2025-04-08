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
        static constexpr const char *k_ZoomValueStrings[] = {"-",  "1/16", "1/8", "1/4", "1/2",  "x1",
                                                             "x2", "x4",   "x8",  "x16", nullptr};
        static constexpr double k_ZoomValues[] = {0., .0625, .125, .25, .5, 1.0, 2.0, 4.0, 8.0, 16.0};

    private:
        int m_PreviewWindowWidth{};
        int m_PreviewWindowHeight{};

        Point<double> m_PanOffset{};
        double m_ZoomFactor{};

        Rect<double> m_ScanArea{};

        uint64_t m_ImageSize{}; // size, in bytes, of the image
        SANE_Byte *m_Image{}; // buffer to hold the scanned data
        uint64_t m_Offset{}; // offset in the image buffer where the next write will be done
        int m_PixelsPerLine{};
        int m_BytesPerLine{};
        int m_ImageHeight{};

        void ApplyPanConstraints()
        {
            if (m_PreviewWindowWidth >= m_PixelsPerLine * m_ZoomFactor)
            {
                m_PanOffset.x = 0;
            }
            else
            {
                if (m_PanOffset.x > 0)
                {
                    m_PanOffset.x = 0;
                }
                else if (m_PanOffset.x < m_PreviewWindowWidth - m_PixelsPerLine * m_ZoomFactor)
                {
                    m_PanOffset.x = m_PreviewWindowWidth - m_PixelsPerLine * m_ZoomFactor;
                }
            }

            if (m_PreviewWindowHeight >= m_ImageHeight * m_ZoomFactor)
            {
                m_PanOffset.y = 0;
            }
            else
            {
                if (m_PanOffset.y > 0)
                {
                    m_PanOffset.y = 0;
                }
                else if (m_PanOffset.y < m_PreviewWindowHeight - m_ImageHeight * m_ZoomFactor)
                {
                    m_PanOffset.y = m_PreviewWindowHeight - m_ImageHeight * m_ZoomFactor;
                }
            }
        }

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

            void SetPreviewWindowSize(int width, int height)
            {
                if (m_StateComponent->m_PreviewWindowWidth == width &&
                    m_StateComponent->m_PreviewWindowHeight == height)
                {
                    return;
                }

                m_StateComponent->m_PreviewWindowWidth = width;
                m_StateComponent->m_PreviewWindowHeight = height;

                m_StateComponent->ApplyPanConstraints();

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::PanOffset);
            }

            void SetPanOffset(Point<double> panOffsetDelta)
            {
                if (m_StateComponent->m_PanOffset == panOffsetDelta)
                {
                    return;
                }

                m_StateComponent->m_PanOffset = panOffsetDelta;
                m_StateComponent->ApplyPanConstraints();

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::PanOffset);
            }

            void SetZoomFactor(double zoomFactor)
            {
                SetZoomFactor(
                        zoomFactor, Point{m_StateComponent->m_PreviewWindowWidth / 2.,
                                          m_StateComponent->m_PreviewWindowHeight / 2.});
            }

            void SetZoomFactor(double zoomFactor, Point<double> zoomCenter)
            {
                zoomFactor = ClampToNearestZoomFactor(zoomFactor);

                if (m_StateComponent->m_ZoomFactor == zoomFactor)
                {
                    return;
                }

                // Adjust pan offset to zoom around the center of the preview window.
                auto centerX = (m_StateComponent->m_PanOffset.x - zoomCenter.x) / m_StateComponent->m_ZoomFactor;
                auto centerY = (m_StateComponent->m_PanOffset.y - zoomCenter.y) / m_StateComponent->m_ZoomFactor;

                m_StateComponent->m_PanOffset.x = zoomFactor * centerX + zoomCenter.x;
                m_StateComponent->m_PanOffset.y = zoomFactor * centerY + zoomCenter.y;

                m_StateComponent->m_ZoomFactor = zoomFactor;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                m_StateComponent->ApplyPanConstraints();
                changeset->Set(PreviewStateChangeset::TypeFlag::PanOffset);
                changeset->Set(PreviewStateChangeset::TypeFlag::ZoomFactor);
            }

            void UpdatePreviewRectangle(const Rect<double> &scanArea)
            {
                if (m_StateComponent->m_ScanArea == scanArea)
                {
                    return;
                }

                m_StateComponent->m_ScanArea = scanArea;
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::ScanArea);
            }

            void PrepareForScan(int pixelsPerLine, int bytesPerLine, int imageHeight)
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

            void GetReadBuffer(SANE_Byte *&buffer, int &maxLength)
            {
                buffer = m_StateComponent->m_Image + m_StateComponent->m_Offset;
                maxLength = static_cast<int>(
                        std::min(1024UL * 1024UL, m_StateComponent->m_ImageSize - m_StateComponent->m_Offset));
            }

            void CommitReadBuffer(int readLength)
            {
                m_StateComponent->m_Offset += readLength;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->Version());
                changeset->Set(PreviewStateChangeset::TypeFlag::Image);
            }

            void ResetReadBuffer()
            {
                m_StateComponent->m_Offset = 0;
            }

            void SetProgressBounds(uint64_t min, uint64_t max)
            {
            }

            void IncreaseProgress(uint64_t delta)
            {
            }

            void SetProgressCompleted()
            {
            }
        };
    };
}
