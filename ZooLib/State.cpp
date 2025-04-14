#include "State.hpp"

#include "StateComponent.hpp"


ZooLib::State::~State()
{
    auto stateComponents = m_StateComponents;
    for (auto &stateComponent: stateComponents)
    {
        delete stateComponent;
    }
}
