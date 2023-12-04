#pragma once

#include <gtk/gtktypes.h>

namespace Zoo
{
    void ShowUserError(GtkWindow *parent, const char *message, ...);
}
