#pragma once

#include <sane/sane.h>

#include "Rect.hpp"
#include "ZooLib/ChangesetBase.hpp"
#include "ZooLib/ChangesetManager.hpp"
#include "ZooLib/StateComponent.hpp"

namespace Gorfector
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
            Progress = 16,
        };

    private:
        std::underlying_type_t<TypeFlag> m_ChangeType{};
        int m_LastLine{-1};

    public:
        explicit PreviewStateChangeset(uint64_t stateInitialVersion)
            : ChangesetBase(stateInitialVersion)
        {
        }

        void Clear()
        {
            m_ChangeType = static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
            m_LastLine = -1;
        }

        void Set(TypeFlag typeFlag, int lastLine = -1)
        {
            m_ChangeType |= static_cast<std::underlying_type_t<TypeFlag>>(typeFlag);
            if (lastLine != -1)
            {
                m_LastLine = lastLine;
            }
        }

        [[nodiscard]] bool HasAnyChange() const
        {
            return m_ChangeType != static_cast<std::underlying_type_t<TypeFlag>>(TypeFlag::None);
        }

        [[nodiscard]] bool IsChanged(TypeFlag typeFlag) const
        {
            return (m_ChangeType & static_cast<std::underlying_type_t<TypeFlag>>(typeFlag)) != 0;
        }

        [[nodiscard]] int GetLastLine() const
        {
            return m_LastLine;
        }

        void Aggregate(const PreviewStateChangeset &changeset)
        {
            ChangesetBase::Aggregate(changeset);

            m_ChangeType |= changeset.m_ChangeType;
            if (changeset.m_LastLine == -1)
            {
                m_LastLine = -1;
            }
            else
            {
                m_LastLine = std::max(m_LastLine, changeset.m_LastLine);
            }
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
        int m_BitDepth{};
        SANE_Frame m_PixelFormat{};

        std::string m_ProgressText;
        uint64_t m_ProgressMin{};
        uint64_t m_ProgressMax{};
        uint64_t m_ProgressCurrent{};

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

        [[nodiscard]] int GetScannedImageBitDepth() const
        {
            return m_BitDepth;
        }

        [[nodiscard]] SANE_Frame GetScannedImagePixelFormat() const
        {
            return m_PixelFormat;
        }

        [[nodiscard]] const SANE_Byte *GetScannedImage() const
        {
            return m_Image;
        }

        [[nodiscard]] int GetScannedBytesPerLine() const
        {
            return m_BytesPerLine;
        }

        [[nodiscard]] const std::string &GetProgressText() const
        {
            return m_ProgressText;
        }

        [[nodiscard]] uint64_t GetProgressMin() const
        {
            return m_ProgressMin;
        }

        [[nodiscard]] uint64_t GetProgressMax() const
        {
            return m_ProgressMax;
        }

        [[nodiscard]] uint64_t GetProgressCurrent() const
        {
            return m_ProgressCurrent;
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

            ~Updater() override
            {
                m_StateComponent->PushCurrentChangeset();
            }

            void LoadFromJson(const nlohmann::json &json) override
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

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
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

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
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

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                m_StateComponent->ApplyPanConstraints();
                changeset->Set(PreviewStateChangeset::TypeFlag::PanOffset);
                changeset->Set(PreviewStateChangeset::TypeFlag::ZoomFactor);
            }

            void UpdateScanArea(const Rect<double> &scanArea)
            {
                if (m_StateComponent->m_ScanArea == scanArea)
                {
                    return;
                }

                m_StateComponent->m_ScanArea = scanArea;
                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::ScanArea);
            }

            void
            PrepareForScan(int pixelsPerLine, int bytesPerLine, int imageHeight, int bitDepth, SANE_Frame pixelFormat)
            {
                m_StateComponent->m_PixelsPerLine = pixelsPerLine;
                m_StateComponent->m_BytesPerLine = bytesPerLine;
                m_StateComponent->m_ImageHeight = imageHeight;
                m_StateComponent->m_BitDepth = bitDepth;
                m_StateComponent->m_PixelFormat = pixelFormat;

                const uint64_t requestedSize = bytesPerLine * imageHeight;
                if (m_StateComponent->m_Image != nullptr && m_StateComponent->m_ImageSize != requestedSize)
                {
                    free(m_StateComponent->m_Image);
                    m_StateComponent->m_Image = nullptr;
                }

                if (m_StateComponent->m_Image == nullptr)
                {
                    m_StateComponent->m_ImageSize = requestedSize;
                    m_StateComponent->m_Image =
                            static_cast<SANE_Byte *>(calloc(m_StateComponent->m_ImageSize, sizeof(SANE_Byte)));
                }

                m_StateComponent->m_Offset = 0;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::Image, -1);
            }

            void GetReadBuffer(SANE_Byte *&buffer, int &maxLength)
            {
                buffer = m_StateComponent->m_Image + m_StateComponent->m_Offset;
                maxLength = static_cast<int>(m_StateComponent->m_ImageSize - m_StateComponent->m_Offset);
            }

            void CommitReadBuffer(int readLength)
            {
                m_StateComponent->m_Offset += readLength;

                int lastWrittenLine =
                        static_cast<int>(m_StateComponent->m_Offset / m_StateComponent->m_BytesPerLine) - 1;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::Image, lastWrittenLine);
            }

            void ResetReadBuffer()
            {
                m_StateComponent->m_Offset = 0;
            }

            void InitProgress(const std::string &text, uint64_t min, uint64_t max)
            {
                m_StateComponent->m_ProgressText = text;
                m_StateComponent->m_ProgressMin = min;
                m_StateComponent->m_ProgressMax = max;
                m_StateComponent->m_ProgressCurrent = min;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::Progress);
            }

            void IncreaseProgress(uint64_t delta)
            {
                m_StateComponent->m_ProgressCurrent += delta;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::Progress);
            }

            void SetProgressCompleted()
            {
                m_StateComponent->m_ProgressText = std::string();
                m_StateComponent->m_ProgressMin = 0UL;
                m_StateComponent->m_ProgressMax = 0UL;
                m_StateComponent->m_ProgressCurrent = 0UL;

                auto changeset = m_StateComponent->GetCurrentChangeset(m_StateComponent->GetVersion());
                changeset->Set(PreviewStateChangeset::TypeFlag::Progress);
            }
        };
    };
}
