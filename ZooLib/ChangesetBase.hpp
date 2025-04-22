#pragma once

#include <algorithm>
#include <cstdint>

namespace ZooLib
{
    class ChangesetBase
    {
        uint64_t m_StateInitialVersion;

    public:
        explicit ChangesetBase(uint64_t stateInitialVersion)
        {
            m_StateInitialVersion = stateInitialVersion;
        }

        [[nodiscard]] uint64_t GetStateInitialVersion() const
        {
            return m_StateInitialVersion;
        }

        void Aggregate(const ChangesetBase &changeset)
        {
            m_StateInitialVersion = std::min(m_StateInitialVersion, changeset.m_StateInitialVersion);
        }
    };
}
