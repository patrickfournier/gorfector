#include "ZooLib/ObserverManager.hpp"
#include "../TestsSupport/LogStateComponent.hpp"
#include "../TestsSupport/Observers.hpp"
#include "../TestsSupport/StateComponents.hpp"
#include "gtest/gtest.h"

using namespace TestsSupport;

namespace ZooLib
{
    class ZooLib_ObserverManagerTestFixture : public testing::Test
    {
    protected:
        State *m_State{};
        StateComponentB *m_DefaultSC{};
        Log *m_Log{};
        ObserverManager *m_ObserverManager{};

        void SetUp() override
        {
            m_State = new State();
            m_DefaultSC = new StateComponentB(m_State);
            m_Log = new Log(m_State);
            m_ObserverManager = new ObserverManager();
        }

        void TearDown() override
        {
            delete m_ObserverManager;
            delete m_Log;
            delete m_DefaultSC;
            delete m_State;
        }
    };

    testing::AssertionResult
    IsPermutation(const char *expr1, const char *expr2, const std::string &str, const std::string &perm)
    {
        if (str.length() == perm.length())
        {
            std::string sortedStr = str;
            std::string sortedPerm = perm;

            std::ranges::sort(sortedStr);
            std::ranges::sort(sortedPerm);

            if (sortedStr == sortedPerm)
            {
                return testing::AssertionSuccess();
            }
        }

        return testing::AssertionFailure()
               << "IsPermutation(" << expr1 << ", " << expr2 << "), actual: " << testing::PrintToString(perm) << " vs "
               << testing::PrintToString(str);
    }

    TEST_F(ZooLib_ObserverManagerTestFixture, ObserversAreRun)
    {
        auto obs1 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'a');
        auto obs2 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'b');
        auto obs3 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'c');

        m_ObserverManager->AddObserver(obs1);
        m_ObserverManager->AddObserver(obs2);
        m_ObserverManager->AddObserver(obs3);

        EXPECT_EQ("", m_Log->GetLog());

        m_ObserverManager->NotifyObservers();

        EXPECT_PRED_FORMAT2(IsPermutation, "abc", m_Log->GetLog());
    }

    TEST_F(ZooLib_ObserverManagerTestFixture, ObserversAreRunOnce)
    {
        auto scA1 = new StateComponentA(m_State);
        auto scA2 = new StateComponentA(m_State);
        auto scA3 = new StateComponentA(m_State);

        auto obs1 = new ObserverThatLogs({scA1, scA2, scA3}, {m_Log}, 'a');

        m_ObserverManager->AddObserver(obs1);

        EXPECT_EQ("", m_Log->GetLog());

        m_ObserverManager->NotifyObservers();

        EXPECT_EQ("a", m_Log->GetLog());

        delete obs1;
    }

    TEST_F(ZooLib_ObserverManagerTestFixture, RemovedObserverIsNotRun)
    {
        auto obs1 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'a');
        auto obs2 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'b');
        auto obs3 = new ObserverThatLogs({m_DefaultSC}, {m_Log}, 'c');

        m_ObserverManager->AddObserver(obs1);
        m_ObserverManager->AddObserver(obs2);
        m_ObserverManager->AddObserver(obs3);

        EXPECT_EQ("", m_Log->GetLog());

        m_ObserverManager->RemoveObserver(obs2);
        m_ObserverManager->NotifyObservers();

        EXPECT_PRED_FORMAT2(IsPermutation, "ac", m_Log->GetLog());

        delete obs1;
        delete obs2;
        delete obs3;
    }

    TEST_F(ZooLib_ObserverManagerTestFixture, DependentObserversAreRunInOrder)
    {
        auto scA = new StateComponentA(m_State);
        auto scB = new StateComponentB(m_State);

        auto obs1 = new ObserverThatLogs({scB}, {m_Log, scA}, 'a');
        auto obs2 = new ObserverThatLogs({scA}, {m_Log}, 'b');
        auto obs3 = new ObserverThatLogs({m_DefaultSC}, {m_Log, scB}, 'c');

        m_ObserverManager->AddObserver(obs1);
        m_ObserverManager->AddObserver(obs2);
        m_ObserverManager->AddObserver(obs3);

        EXPECT_EQ("", m_Log->GetLog());

        m_ObserverManager->NotifyObservers();

        EXPECT_EQ("cab", m_Log->GetLog());

        delete scA;
        delete scB;

        delete obs1;
        delete obs2;
        delete obs3;
    }

    TEST_F(ZooLib_ObserverManagerTestFixture, ObserverDeletedByAnotherObserverIsNotRun)
    {
        auto scA = new StateComponentA(m_State);
        auto scB = new StateComponentB(m_State);

        auto obs2 = new ObserverThatLogs({scB}, {}, 'a');
        auto obs1 = new ObserverThatDeletes({scA}, {scB}, m_ObserverManager, obs2);

        m_ObserverManager->AddObserver(obs1);
        m_ObserverManager->AddObserver(obs2);

        m_ObserverManager->NotifyObservers();

        EXPECT_EQ("", m_Log->GetLog());

        delete scA;
        delete scB;

        delete obs1;
    }
}
