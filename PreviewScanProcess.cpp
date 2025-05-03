#include "PreviewScanProcess.hpp"

void Gorfector::PreviewScanProcess::SetPreviewOptions() const
{
    if (m_Device == nullptr || m_ScanOptions == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_TRUE;
    m_Device->SetOptionValue(m_ScanOptions->PreviewIndex(), &isPreview, nullptr);

    if (m_ScanOptions->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto resolutionDescription = m_Device->GetOptionDescriptor(m_ScanOptions->ResolutionIndex());
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
                auto diff = std::abs(resolution - resolutionDescription->constraint.word_list[1]);
                if (diff < smallestDiff)
                {
                    nearestResolutionIndex = i;
                    smallestDiff = diff;
                }
            }
            resolution = resolutionDescription->constraint.word_list[nearestResolutionIndex];
        }

        m_Device->SetOptionValue(m_ScanOptions->ResolutionIndex(), &resolution, nullptr);
    }

    if (m_ScanOptions->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto depthDescription = m_Device->GetOptionDescriptor(m_ScanOptions->BitDepthIndex());

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

            m_Device->SetOptionValue(m_ScanOptions->BitDepthIndex(), &depth, nullptr);
        }
    }

    auto maxScanArea = m_ScanOptions->GetMaxScanArea();
    if (m_ScanOptions->TLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = m_Device->GetOptionDescriptor(m_ScanOptions->TLXIndex());
        double value = maxScanArea.x;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->TLXIndex(), &fValue, nullptr);
    }
    if (m_ScanOptions->TLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = m_Device->GetOptionDescriptor(m_ScanOptions->TLYIndex());
        double value = maxScanArea.y;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->TLYIndex(), &fValue, nullptr);
    }
    if (m_ScanOptions->BRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = m_Device->GetOptionDescriptor(m_ScanOptions->BRXIndex());
        double value = maxScanArea.x + maxScanArea.width;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->BRXIndex(), &fValue, nullptr);
    }
    if (m_ScanOptions->BRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto description = m_Device->GetOptionDescriptor(m_ScanOptions->BRYIndex());
        double value = maxScanArea.y + maxScanArea.height;
        SANE_Int fValue = description->type == SANE_TYPE_FIXED ? SANE_FIX(value) : static_cast<SANE_Int>(value);
        m_Device->SetOptionValue(m_ScanOptions->BRYIndex(), &fValue, nullptr);
    }
}

void Gorfector::PreviewScanProcess::RestoreOptionsAfterPreview() const
{
    if (m_Device == nullptr || m_ScanOptions == nullptr)
    {
        return;
    }

    SANE_Bool isPreview = SANE_FALSE;
    m_Device->SetOptionValue(m_ScanOptions->PreviewIndex(), &isPreview, nullptr);

    if (m_ScanOptions->XResolutionIndex() == std::numeric_limits<uint32_t>::max())
    {
        if (m_ScanOptions->ResolutionIndex() != std::numeric_limits<uint32_t>::max())
        {
            auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->ResolutionIndex());
            int value = option->GetValue();
            m_Device->SetOptionValue(m_ScanOptions->ResolutionIndex(), &value, nullptr);
        }
    }
    else
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->XResolutionIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->XResolutionIndex(), &value, nullptr);
    }

    if (m_ScanOptions->YResolutionIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->YResolutionIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->YResolutionIndex(), &value, nullptr);
    }

    if (m_ScanOptions->BitDepthIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->BitDepthIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->BitDepthIndex(), &value, nullptr);
    }

    if (m_ScanOptions->TLXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->TLXIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->TLXIndex(), &value, nullptr);
    }
    if (m_ScanOptions->TLYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->TLYIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->TLYIndex(), &value, nullptr);
    }
    if (m_ScanOptions->BRXIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->BRXIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->BRXIndex(), &value, nullptr);
    }
    if (m_ScanOptions->BRYIndex() != std::numeric_limits<uint32_t>::max())
    {
        auto option = m_ScanOptions->GetOption<int>(m_ScanOptions->BRYIndex());
        int value = option->GetValue();
        m_Device->SetOptionValue(m_ScanOptions->BRYIndex(), &value, nullptr);
    }
}
