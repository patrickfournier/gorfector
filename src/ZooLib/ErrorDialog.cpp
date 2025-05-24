#include <string>

#include "ErrorDialog.hpp"
#include "Gettext.hpp"

namespace ZooLib
{
    void ShowUserError(AdwApplicationWindow *parentWindow, const std::string &message)
    {
        auto dialog = adw_alert_dialog_new(_("Error"), nullptr);
        adw_alert_dialog_set_body(ADW_ALERT_DIALOG(dialog), message.c_str());
        adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), "ok", "OK");
        adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "ok");

        adw_dialog_present(ADW_DIALOG(dialog), GTK_WIDGET(parentWindow));
    }
}
