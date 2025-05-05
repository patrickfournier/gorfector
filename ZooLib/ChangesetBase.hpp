#pragma once

#include <algorithm>

namespace ZooLib
{
    /**
     * \brief Base class representing a changeset for tracking state changes.
     * \details This class provides functionality to keep track of the changes that have occurred since some version
     * of the state. Derived class should store specific changes and reimplement the Aggregate method.
     */
    class ChangesetBase
    {
        /**
         * \brief The initial version of the state when the changeset was created.
         */
        uint64_t m_StateInitialVersion;

    public:
        /**
         * \brief Constructs a ChangesetBase with the given initial state version.
         * \param stateInitialVersion The initial version of the state.
         */
        explicit ChangesetBase(uint64_t stateInitialVersion)
        {
            m_StateInitialVersion = stateInitialVersion;
        }

        /**
         * \brief Retrieves the initial version of the state.
         * \return The initial version of the state.
         */
        [[nodiscard]] uint64_t GetStateInitialVersion() const
        {
            return m_StateInitialVersion;
        }

        /**
         * \brief Aggregates another changeset into this one by updating the initial version.
         * \details The initial version is updated to the minimum of the current and the other changeset's initial
         * version.
         * \param changeset The changeset to aggregate.
         * \note Derived class should implement a specific Aggregate method that takes a derived changeset as its
         * parameter. The method in the derived class should call this method to ensure the initial version is updated
         * correctly.
         */
        void Aggregate(const ChangesetBase &changeset)
        {
            m_StateInitialVersion = std::min(m_StateInitialVersion, changeset.m_StateInitialVersion);
        }
    };
}
