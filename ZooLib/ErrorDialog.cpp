#include "ErrorDialog.hpp"

#include "View.hpp"

namespace ZooLib
{
    void ShowUserError(AdwApplicationWindow *parentWindow, const char *format, ...)
    {
        auto dialog = adw_alert_dialog_new("Error", nullptr);

        va_list args;
        va_start(args, format);
        adw_alert_dialog_format_body(ADW_ALERT_DIALOG(dialog), format, args);
        va_end(args);

        adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), "ok", "OK");
        adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "ok");

        adw_alert_dialog_choose(ADW_ALERT_DIALOG(dialog), GTK_WIDGET(parentWindow), nullptr, nullptr, nullptr);
    }
}
