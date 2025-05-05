#pragma once

#include <adwaita.h>
#include <string>

namespace ZooLib
{
    /**
     * \brief Displays an error dialog to the user.
     *
     * This function shows a modal error dialog with the specified message.
     *
     * \param parentWindow A pointer to the parent `AdwApplicationWindow` where the dialog will be displayed.
     * \param message The error message to display in the dialog.
     */
    void ShowUserError(AdwApplicationWindow *parentWindow, const std::string &message);
}
