#include "PreviewScanProcess.hpp"

void Gorfector::PreviewScanProcess::SetPreviewOptions() const
{
    if (m_Device == nullptr || m_ScanOptions == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_TRUE;
    m_Device->SetOptionValue(m_ScanOptions->GetPreviewIndex(), &isPreview, nullptr);

    const SANE_Option_Descriptor *resolutionDescription = nullptr;
    if (m_ScanOptions->GetResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        resolutionDescription = m_Device->GetOptionDescriptor(m_ScanOptions->GetResolutionIndex());
    }
    if (resolutionDescription != nullptr)
    {
        SANE_Int resolution = resolutionDescription->type == SANE_TYPE_FIXED ? SANE_FIX(300) : 300;
        if (resolutionDescription->constraint_type == SANE_CONSTRAINT_RANGE)
        {
            resolution = std::clamp(
                    resolution, resolutionDescription->constraint.range->min,
                    resolutionDescription->constraint.range->max);
        }
        else if (resolutionDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
        {
            auto count = resolutionDescription->constraint.word_list[0];
            int nearestResolutionIndex = 1;
            SANE_Int smallestDiff = std::abs(resolution - resolutionDescription->constraint.word_list[1]);
            for (auto i = 2; i <= count; i++)
            {
                auto diff = std::abs(resolution - resolutionDescription->constraint.word_list[i]);
                if (diff < smallestDiff)
                {
                    nearestResolutionIndex = i;
                    smallestDiff = diff;
                }
            }
            resolution = resolutionDescription->constraint.word_list[nearestResolutionIndex];
        }

        m_Device->SetOptionValue(m_ScanOptions->GetResolutionIndex(), &resolution, nullptr);
    }

    const SANE_Option_Descriptor *depthDescription = nullptr;
    if (m_ScanOptions->GetBitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        depthDescription = m_Device->GetOptionDescriptor(m_ScanOptions->GetBitDepthIndex());
    }
    if (depthDescription != nullptr)
    {
        if (!(depthDescription->cap & SANE_CAP_INACTIVE))
        {
            SANE_Int depth = depthDescription->type == SANE_TYPE_FIXED ? SANE_FIX(8) : 8;
            if (depthDescription->constraint_type == SANE_CONSTRAINT_RANGE)
            {
                depth = std::clamp(
                        depth, depthDescription->constraint.range->min, depthDescription->constraint.range->max);
            }
            else if (depthDescription->constraint_type == SANE_CONSTRAINT_WORD_LIST)
            {
                auto count = depthDescription->constraint.word_list[0];
                SANE_Int minDepth = depthDescription->constraint.word_list[1];
                for (auto i = 1; i <= count; i++)
                {
                    if (depthDescription->constraint.word_list[i] == depth)
                    {
                        // If 8 bits is there, we take it.
                        break;
                    }

                    minDepth = std::min(minDepth, depthDescription->constraint.word_list[i]);
                }
            }

            m_Device->SetOptionValue(m_ScanOptions->GetBitDepthIndex(), &depth, nullptr);
        }
    }

    const SANE_Option_Descriptor *description = nullptr;
    auto maxScanArea = m_ScanOptions->GetMaxScanArea();
    if (m_ScanOptions->GetTLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        description = m_Device->GetOptionDescriptor(m_ScanOptions->GetTLXIndex());
    }
    if (description != nullptr)
    {
        double value = maxScanArea.x;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->GetTLXIndex(), &fValue, nullptr);
    }

    if (m_ScanOptions->GetTLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        description = m_Device->GetOptionDescriptor(m_ScanOptions->GetTLYIndex());
    }
    if (description != nullptr)
    {
        double value = maxScanArea.y;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->GetTLYIndex(), &fValue, nullptr);
    }

    if (m_ScanOptions->GetBRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        description = m_Device->GetOptionDescriptor(m_ScanOptions->GetBRXIndex());
    }
    if (description != nullptr)
    {
        double value = maxScanArea.x + maxScanArea.width;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->GetBRXIndex(), &fValue, nullptr);
    }

    if (m_ScanOptions->GetBRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        description = m_Device->GetOptionDescriptor(m_ScanOptions->GetBRYIndex());
    }
    if (description != nullptr)
    {
        double value = maxScanArea.y + maxScanArea.height;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->GetBRYIndex(), &fValue, nullptr);
    }
}

void Gorfector::PreviewScanProcess::RestoreOptionsAfterPreview() const
{
    if (m_Device == nullptr || m_ScanOptions == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_FALSE;
    m_Device->SetOptionValue(m_ScanOptions->GetPreviewIndex(), &isPreview, nullptr);

    if (m_ScanOptions->GetXResolutionIndex() == std::numeric_limits<uint32_t>::max())
    {
        if (m_ScanOptions->GetResolutionIndex() != std::numeric_limits<uint32_t>::max())
        {
            auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetResolutionIndex());
            int value = option->GetValue();
            m_Device->SetOptionValue(m_ScanOptions->GetResolutionIndex(), &value, nullptr);
        }
    }
    else
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetXResolutionIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetXResolutionIndex(), &value, nullptr);
    }

    if (m_ScanOptions->GetYResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetYResolutionIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetYResolutionIndex(), &value, nullptr);
    }

    if (m_ScanOptions->GetBitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetBitDepthIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetBitDepthIndex(), &value, nullptr);
    }

    if (m_ScanOptions->GetTLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetTLXIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetTLXIndex(), &value, nullptr);
    }
    if (m_ScanOptions->GetTLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetTLYIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetTLYIndex(), &value, nullptr);
    }
    if (m_ScanOptions->GetBRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetBRXIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetBRXIndex(), &value, nullptr);
    }
    if (m_ScanOptions->GetBRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->GetBRYIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->GetBRYIndex(), &value, nullptr);
    }
}
