#pragma once
#include "ZooLib/ChangesetBase.hpp"

namespace TestsSupport
{
    class ChangesetA : public ZooLib::ChangesetBase
    {
    public:
        using ChangesetBase::ChangesetBase;
    };
}
