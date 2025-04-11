#pragma once

#include <gtk/gtktypes.h>

namespace ZooLib
{
    void ShowUserError(GtkWindow *parentWindow, const char *message, ...);
}
