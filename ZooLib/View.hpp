#pragma once

#include <gtk/gtk.h>

namespace ZooLib
{

    class View
    {
    public:
        virtual ~View() = default;

        [[nodiscard]] virtual GtkWidget *GetRootWidget() const = 0;

        virtual void Update(uint64_t lastSeenVersion) = 0;
    };

}
