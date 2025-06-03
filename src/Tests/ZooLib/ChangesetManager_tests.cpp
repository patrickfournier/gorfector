#include "gtest/gtest.h"

#include "../TestsSupport/Changesets.hpp"
#include "ZooLib/ChangesetManager.hpp"

namespace ZooLib
{
    TEST(ZooLib_ChangesetManagerTests, CorrectlyCreatesAnInitialChangeset)
    {
        ChangesetManager<TestsSupport::ChangesetA> changesetManager;
        auto changeset = changesetManager.GetCurrentChangeset(1);

        EXPECT_NE(changeset, nullptr);
        EXPECT_EQ(changeset->GetStateInitialVersion(), 1);
    }

    TEST(ZooLib_ChangesetManagerTests, CorrectlyPushesChangeset)
    {
        ChangesetManager<TestsSupport::ChangesetA> changesetManager;
        auto changeset = changesetManager.GetCurrentChangeset(1); // side effect: create changeset
        changesetManager.PushCurrentChangeset();
        changeset = changesetManager.GetCurrentChangeset(2);

        EXPECT_NE(changeset, nullptr);
        EXPECT_EQ(changeset->GetStateInitialVersion(), 2);
        EXPECT_EQ(changesetManager.FirstChangesetVersion(), 1);
    }

    TEST(ZooLib_ChangesetManagerTests, CorrectlyAggregatesChangesets)
    {
        ChangesetManager<TestsSupport::ChangesetA> changesetManager;
        auto changeset1 = changesetManager.GetCurrentChangeset(1); // side effect: create changeset
        changesetManager.PushCurrentChangeset();

        auto changeset2 = changesetManager.GetCurrentChangeset(2); // side effect: create changeset
        changesetManager.PushCurrentChangeset();

        auto aggregatedChangeset = changesetManager.GetAggregatedChangeset(1);

        EXPECT_NE(aggregatedChangeset, nullptr);
        EXPECT_EQ(aggregatedChangeset->GetStateInitialVersion(), 1);
    }
}
