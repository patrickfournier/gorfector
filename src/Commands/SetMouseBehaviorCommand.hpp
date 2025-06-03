#pragma once
#include "Command.hpp"
#include "PreviewState.hpp"

namespace Gorfector
{
    class SetMouseBehaviorCommand : public ZooLib::Command
    {
        const PreviewState::MouseBehavior m_MouseBehavior{};

    public:
        SetMouseBehaviorCommand(PreviewState::MouseBehavior mouseBehavior)
            : m_MouseBehavior(mouseBehavior)
        {
        }

        static void Execute(const SetMouseBehaviorCommand &command, PreviewState *previewState)
        {
            auto updater = PreviewState::Updater(previewState);
            updater.SetDefaultMouseBehavior(command.m_MouseBehavior);
        }
    };
}
