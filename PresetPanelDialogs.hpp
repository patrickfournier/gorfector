#pragma once

#include <gtk/gtk.h>

#include "ZooLib/Gettext.hpp"

namespace Gorfector
{
    static const char *s_EmptyNameHint = N_("Preset name must not be empty.");
    static const char *s_NameConflictHint = N_("A preset with this name already exists.");

    static const char *s_ButtonClasses[] = {"raised", nullptr};
    static const char *s_EntryHintClasses[] = {"entry-hint", "error", nullptr};
    static const char *s_TitleClasses[] = {"heading", nullptr};

    void ShowCreatePresetDialog(GtkWidget *widget, gpointer userData);
    void ShowRenamePresetDialog(GtkWidget *widget, gpointer userData);
    void ShowViewPresetDialog(GtkWidget *widget, gpointer userData);
}
