#pragma once

namespace ZooLib
{
    /// \brief Base class for all commands in the ZooLib framework.
    ///
    /// This class serves as a common interface for commands that can be executed
    /// within the application. Derived classes should implement specific command
    /// logic and provide an `Execute` static method to perform the desired action.
    class Command
    {
    };
}
