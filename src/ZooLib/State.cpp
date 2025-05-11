#include "State.hpp"
#include "StateComponent.hpp"

ZooLib::State::~State()
{
    // Use a copy of the state components vector because the destructor of StateComponent will modify it.
    const auto stateComponents = m_StateComponents;
    for (const auto &stateComponent: stateComponents)
    {
        delete stateComponent;
    }
}
