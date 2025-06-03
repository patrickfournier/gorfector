#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace ZooLib
{
    class ChangesetBase;

    /**
     * \class ChangesetManagerBase
     * \brief Base class for managing changesets.
     *
     * This class provides an interface for purging changesets based on their version.
     * It is intended to be inherited by specific changeset manager implementations.
     */
    class ChangesetManagerBase
    {
    public:
        virtual ~ChangesetManagerBase() = default;

        /**
         * \brief Purges all changesets that are older than the specified version.
         * \param untilVersion The first version to keep in the changeset stack.
         */
        virtual void PurgeChangesets(uint64_t untilVersion) = 0;
    };

    /**
     * \class ChangesetManager
     * \brief Manages a collection of changesets and provides functionality to aggregate and retrieve them.
     *
     * \tparam TChangeset The type of changeset managed by this class. Must derive from ChangesetBase.
     *
     * This class is responsible for managing changesets, including creating, storing, and aggregating them.
     * It ensures proper cleanup of dynamically allocated changesets and provides utility methods to interact
     * with the changesets.
     */
    template<typename TChangeset>
    class ChangesetManager final : public ChangesetManagerBase
    {
        /**
         * \brief Pointer to the current changeset being managed.
         */
        TChangeset *m_CurrentChangeset{};

        /**
         * \brief A stack of all pushed changesets.
         */
        std::vector<TChangeset *> m_Changesets;

    public:
        /**
         * \brief Destructor. Cleans up all dynamically allocated changesets.
         */
        ~ChangesetManager() override
        {
            for (auto changeset: m_Changesets)
            {
                delete changeset;
            }

            delete m_CurrentChangeset;
        }

        /**
         * \brief Retrieves the current changeset, creating it if necessary.
         *
         * \param stateComponentVersion The version to initialize the changeset with if it is created.
         * \return A pointer to the current changeset.
         */
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

        /**
         * \brief Pushes the current changeset on the stack and resets the current changeset pointer.
         */
        void PushCurrentChangeset()
        {
            if (m_CurrentChangeset != nullptr)
            {
                m_Changesets.push_back(m_CurrentChangeset);
                m_CurrentChangeset = nullptr;
            }
        }

        /**
         * \brief Aggregates all changesets since a given version into a new changeset.
         *
         * \param sinceVersion The version from which to start aggregating changesets.
         * \return A pointer to the aggregated changeset.
         */
        [[nodiscard]] TChangeset *GetAggregatedChangeset(uint64_t sinceVersion) const
        {
            static_assert(
                    std::is_base_of_v<ChangesetBase, TChangeset>,
                    "The type parameter of ChangesetManager<T> must derive from ChangesetBase");

            auto *aggregatedChangeset = new TChangeset(sinceVersion);

            for (auto changeset: m_Changesets)
            {
                if (changeset->GetStateInitialVersion() >= sinceVersion)
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

        /**
         * \brief Retrieves the version of the changeset at the bottom of the changeset stack.
         *
         * \return The version of the first changeset, or the maximum uint64_t value if the stack is empty.
         */
        [[nodiscard]] uint64_t FirstChangesetVersion() const
        {
            if (m_Changesets.empty())
            {
                return std::numeric_limits<uint64_t>::max();
            }

            return m_Changesets.front()->GetStateInitialVersion();
        }

        /**
         * \brief Purges all changesets that are older than the specified version.
         * \param untilVersion The first version to keep in the changeset stack.
         */
        void PurgeChangesets(uint64_t untilVersion) override
        {
            if (m_Changesets.empty())
            {
                return;
            }

            // Find the first changeset that is newer than, or at, the specified version.
            auto keep =
                    std::find_if(m_Changesets.begin(), m_Changesets.end(), [untilVersion](const TChangeset *changeset) {
                        return changeset->GetStateInitialVersion() >= untilVersion;
                    });

            if (keep == m_Changesets.begin())
            {
                return;
            }

            if (keep != m_Changesets.end())
            {
                // If strictly greater than, `untilVersion` is part of the previous changeset, so we need to keep it.
                if ((*keep)->GetStateInitialVersion() > untilVersion)
                {
                    keep = keep - 1;
                }
            }
            else
            {
                // keep == m_Changesets.end(); the next changeset is m_CurrentChangeset

                if (m_CurrentChangeset == nullptr || m_CurrentChangeset->GetStateInitialVersion() > untilVersion)
                {
                    keep = keep - 1;
                }
            }

            if (keep == m_Changesets.begin())
            {
                return;
            }

            // Delete all changesets that are older than the changeset to keep.
            std::for_each(m_Changesets.begin(), keep, [](TChangeset *changeset) { delete changeset; });
            m_Changesets.erase(m_Changesets.begin(), keep);
        }
    };
}
