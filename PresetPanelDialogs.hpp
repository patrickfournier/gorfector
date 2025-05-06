#pragma once

#include <gtk/gtk.h>

#include "ZooLib/Gettext.hpp"

namespace Gorfector
{
    /**
     * \brief Preset panel dialog constants and functions.
     *
     * This namespace contains constants and function declarations related to the preset panel dialogs.
     */
    static const char *s_EmptyNameHint = N_("Preset name must not be empty."); ///< Hint for empty preset name.
    static const char *s_NameConflictHint = N_("A preset with this name already exists."); ///< Hint for name conflict.

    static const char *s_ButtonClasses[] = {"raised", nullptr}; ///< Classes for styling the dialog buttons.
    static const char *s_EntryHintClasses[] = {"entry-hint", "error", nullptr}; ///< Classes for styling the entry hint.
    static const char *s_TitleClasses[] = {"heading", nullptr}; ///< Classes for styling the dialog title.

    /**
     * \brief Shows the create preset dialog.
     *
     * This function creates and displays a dialog for creating a new preset.
     *
     * \param widget The widget that triggered the dialog.
     * \param userData Pointer to user data (usually the preset panel).
     */
    void ShowCreatePresetDialog(GtkWidget *widget, gpointer userData);

    /**
     * \brief Shows the rename preset dialog.
     *
     * This function creates and displays a dialog for renaming a preset.
     *
     * \param widget The widget that triggered the dialog.
     * \param userData Pointer to user data (usually the preset panel).
     */
    void ShowRenamePresetDialog(GtkWidget *widget, gpointer userData);

    /**
     * \brief Shows the view preset dialog.
     *
     * This function creates and displays a dialog for viewing a preset.
     *
     * \param widget The widget that triggered the dialog.
     * \param userData Pointer to user data (usually the preset panel).
     */
    void ShowViewPresetDialog(GtkWidget *widget, gpointer userData);
}
