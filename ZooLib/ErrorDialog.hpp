#pragma once

#include <gtk/gtktypes.h>

namespace ZooLib
{
    void ShowUserError(GtkWindow *parent, const char *message, ...);
}
