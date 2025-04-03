#pragma once

#include <cstdint>
#include <vector>

namespace ZooLib
{
    class ChangesetBase;

    template<typename TChangeset>
    class ChangesetManager
    {
        TChangeset *m_CurrentChangeset{};
        std::vector<TChangeset *> m_Changesets;

    protected:
        ~ChangesetManager()
        {
            for (auto changeset: m_Changesets)
            {
                delete changeset;
            }

            delete m_CurrentChangeset;
        }

        [[nodiscard]] TChangeset *GetCurrentChangeset(uint64_t stateComponentVersion)
        {
            static_assert(
                    std::is_base_of_v<ChangesetBase, TChangeset>,
                    "The type parameter of ChangesetManager<T> must derive from ChangesetBase");

            if (m_CurrentChangeset == nullptr)
            {
                m_CurrentChangeset = new TChangeset(stateComponentVersion);
            }

            return m_CurrentChangeset;
        }

        void PushCurrentChangeset()
        {
            if (m_CurrentChangeset != nullptr)
            {
                m_Changesets.push_back(m_CurrentChangeset);
                m_CurrentChangeset = nullptr;
            }
        }

    public:
        [[nodiscard]] TChangeset *GetAggregatedChangeset(uint64_t sinceVersion) const
        {
            static_assert(
                    std::is_base_of_v<ChangesetBase, TChangeset>,
                    "The type parameter of ChangesetManager<T> must derive from ChangesetBase");

            auto *aggregatedChangeset = new TChangeset(sinceVersion);

            for (auto changeset: m_Changesets)
            {
                if (changeset->StateInitialVersion() >= sinceVersion)
                {
                    aggregatedChangeset->Aggregate(*changeset);
                }
            }

            if (m_CurrentChangeset != nullptr)
            {
                aggregatedChangeset->Aggregate(*m_CurrentChangeset);
            }

            return aggregatedChangeset;
        }

        [[nodiscard]] uint64_t FirstChangesetVersion() const
        {
            if (m_Changesets.empty())
            {
                return std::numeric_limits<uint64_t>::max();
            }

            return m_Changesets.front()->StateInitialVersion();
        }
    };
}
