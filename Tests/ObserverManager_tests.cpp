#include "gtest/gtest.h"
#include "ZooLib/ObserverManager.hpp"

namespace
{
    class Log : public ZooLib::StateComponent
    {
        std::string m_Log{};

    public:
        Log() = delete;

        explicit Log(ZooLib::State *state)
            : StateComponent{state}
        {
        }

        void Push(char c)
        {
            m_Log.append(1, c);
        }

        const std::string& GetLog() const
        {
            return m_Log;
        }
    };

    class DefaultObservedComponent : public ZooLib::StateComponent
    {
    public:
        explicit DefaultObservedComponent(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    class StateComponentA : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentA(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    class StateComponentB : public ZooLib::StateComponent
    {
    public:
        explicit StateComponentB(ZooLib::State *state)
            : StateComponent{state}
        {
        }
    };

    class TestObserver : public ZooLib::Observer
    {
        char m_Id{};

    protected:
        void UpdateImplementation() override
        {
            auto log = dynamic_cast<Log *>(m_ModifiedComponents[0]);
            log->Push(m_Id);
        }

    public:
        TestObserver(std::vector<const ZooLib::StateComponent *> observedComponents,
                     std::vector<ZooLib::StateComponent *> modifiedComponents, char id)
        : Observer(std::move(observedComponents), std::move(modifiedComponents))
        , m_Id(id)
        {
        }
    };

    class ObserverManagerTestFixture : public ::testing::Test
    {
    protected:
        ZooLib::State *m_State{};
        DefaultObservedComponent *m_DefaultObs{};
        Log *m_Log{};
        ZooLib::ObserverManager *m_ObserverManager{};

        void SetUp() override
        {
            m_State = new ZooLib::State();
            m_DefaultObs = new DefaultObservedComponent(m_State);
            m_Log = new Log(m_State);
            m_ObserverManager = new ZooLib::ObserverManager();
        }

        void TearDown() override
        {
            delete m_ObserverManager;
            delete m_Log;
            delete m_DefaultObs;
            delete m_State;
        }

    };

    testing::AssertionResult IsPermutation(const char* expr1, const char* expr2,
                                           const std::string &str, const std::string &perm)
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

        return testing::AssertionFailure() << "IsPermutation(" << expr1 << ", " << expr2
            << "), actual: " << ::testing::PrintToString(perm)
            << " vs " << ::testing::PrintToString(str);
    }

    TEST_F(ObserverManagerTestFixture, ObserversAreRun)
    {
        auto obs1 = new TestObserver({m_DefaultObs}, {m_Log}, 'a');
        auto obs2 = new TestObserver({m_DefaultObs}, {m_Log}, 'b');
        auto obs3 = new TestObserver({m_DefaultObs}, {m_Log}, 'c');

        m_ObserverManager->AddObserver(obs1);
        m_ObserverManager->AddObserver(obs2);
        m_ObserverManager->AddObserver(obs3);

        EXPECT_EQ("", m_Log->GetLog());

        m_ObserverManager->NotifyObservers();

        EXPECT_PRED_FORMAT2(IsPermutation, "abc", m_Log->GetLog());

        delete obs1;
        delete obs2;
        delete obs3;
    }

    TEST_F(ObserverManagerTestFixture, RemovedObserverIsNotRun)
    {
        auto obs1 = new TestObserver({m_DefaultObs}, {m_Log}, 'a');
        auto obs2 = new TestObserver({m_DefaultObs}, {m_Log}, 'b');
        auto obs3 = new TestObserver({m_DefaultObs}, {m_Log}, 'c');

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

    TEST_F(ObserverManagerTestFixture, DependentObserversAreRunInOrder)
    {
        auto scA = new StateComponentA(m_State);
        auto scB = new StateComponentB(m_State);

        auto obs1 = new TestObserver({scB}, {m_Log, scA}, 'a');
        auto obs2 = new TestObserver({scA}, {m_Log}, 'b');
        auto obs3 = new TestObserver({m_DefaultObs}, {m_Log, scB}, 'c');

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
}
