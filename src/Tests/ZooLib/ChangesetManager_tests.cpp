#include "gtest/gtest.h"

#include "../TestsSupport/Changesets.hpp"
#include "ZooLib/ChangesetManager.hpp"

namespace ZooLib
{
    class TestChangesetManager : public ChangesetManager<TestsSupport::ChangesetA>
    {
    public:
        TestChangesetManager() = default;
        ~TestChangesetManager() override = default;

        TestsSupport::ChangesetA *GetChangeset(uint64_t version)
        {
            auto cs = GetCurrentChangeset(version);
            return cs;
        }

        void PushChangeset()
        {
            PushCurrentChangeset();
        }
    };

    TEST(ZooLib_ChangesetManagerTests, CorrectlyCreatesAnInitialChangeset)
    {
        TestChangesetManager changesetManager;
        auto changeset = changesetManager.GetChangeset(1);

        EXPECT_NE(changeset, nullptr);
        EXPECT_EQ(changeset->GetStateInitialVersion(), 1);
    }

    TEST(ZooLib_ChangesetManagerTests, CorrectlyPushesChangeset)
    {
        TestChangesetManager changesetManager;
        changesetManager.GetChangeset(1); // side effect: create changeset
        changesetManager.PushChangeset();
        auto changeset = changesetManager.GetChangeset(2);

        EXPECT_NE(changeset, nullptr);
        EXPECT_EQ(changeset->GetStateInitialVersion(), 2);
        EXPECT_EQ(changesetManager.FirstChangesetVersion(), 1);
    }

    TEST(ZooLib_ChangesetManagerTests, CorrectlyAggregatesChangesets)
    {
        TestChangesetManager changesetManager;
        auto changeset1 = changesetManager.GetChangeset(1);
        changesetManager.PushChangeset();

        auto changeset2 = changesetManager.GetChangeset(2);
        changesetManager.PushChangeset();

        auto aggregatedChangeset = changesetManager.GetAggregatedChangeset(1);

        EXPECT_NE(aggregatedChangeset, nullptr);
        EXPECT_EQ(aggregatedChangeset->GetStateInitialVersion(), 1);
    }
}
