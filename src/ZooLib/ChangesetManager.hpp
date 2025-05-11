#pragma once

#include <vector>

namespace ZooLib
{
    class ChangesetBase;

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
    class ChangesetManager
    {
        /**
         * \brief Pointer to the current changeset being managed.
         */
        TChangeset *m_CurrentChangeset{};

        /**
         * \brief A stack of all pushed changesets.
         */
        std::vector<TChangeset *> m_Changesets;

    protected:
        /**
         * \brief Destructor. Cleans up all dynamically allocated changesets.
         */
        virtual ~ChangesetManager()
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

    public:
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
    };
}
