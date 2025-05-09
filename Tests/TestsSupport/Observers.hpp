#pragma once
#include <vector>

#include "LogStateComponent.hpp"
#include "ZooLib/Observer.hpp"
#include "ZooLib/StateComponent.hpp"

namespace TestsSupport
{
    class TestObserver : public ZooLib::Observer
    {
    protected:
        void UpdateImplementation() override
        {
        }

    public:
        // The first modifiedComponents should be a Log.
        TestObserver(
                std::vector<const ZooLib::StateComponent *> observedComponents,
                std::vector<ZooLib::StateComponent *> modifiedComponents)
            : Observer(std::move(observedComponents), std::move(modifiedComponents))
        {
        }
    };

    class ObserverThatLogs : public ZooLib::Observer
    {
        char m_Id{};

    protected:
        void UpdateImplementation() override
        {
            auto log = dynamic_cast<Log *>(m_ModifiedComponents[0]);
            log->Push(m_Id);
        }

    public:
        // The first modifiedComponents should be a Log.
        ObserverThatLogs(
                std::vector<const ZooLib::StateComponent *> observedComponents,
                std::vector<ZooLib::StateComponent *> modifiedComponents, char id)
            : Observer(std::move(observedComponents), std::move(modifiedComponents))
            , m_Id(id)
        {
        }
    };

    class ObserverThatDeletes : public ZooLib::Observer
    {
        ZooLib::ObserverManager *m_ObserverManager{};
        Observer *m_ObserverToDelete{};

    protected:
        void UpdateImplementation() override
        {
            m_ObserverManager->RemoveObserver(m_ObserverToDelete);
            delete m_ObserverToDelete;
        }

    public:
        // The first modifiedComponents should be a Log.
        ObserverThatDeletes(
                std::vector<const ZooLib::StateComponent *> observedComponents,
                std::vector<ZooLib::StateComponent *> modifiedComponents, ZooLib::ObserverManager *observerManager,
                Observer *observerToDelete)
            : Observer(std::move(observedComponents), std::move(modifiedComponents))
            , m_ObserverManager(observerManager)
            , m_ObserverToDelete(observerToDelete)
        {
        }
    };
}
