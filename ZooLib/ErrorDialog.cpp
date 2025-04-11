#include <gtk/gtk.h>

#include "ModalDialog.hpp"

namespace ZooLib
{
    class ErrorView : public View
    {
        GtkWidget *m_RootWidget;

    public:
        ErrorView(const char *message)
        {
            m_RootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            auto label = gtk_label_new(message);
            gtk_box_append(GTK_BOX(m_RootWidget), label);
        }

        ~ErrorView() override = default;

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        void Update(uint64_t lastSeenVersion) override
        {
            // No update needed for the error dialog
        }
    };

    void ShowUserError(GtkWindow *parentWindow, const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        char message[1024];
        vsnprintf(message, sizeof(message), format, args);
        va_end(args);

        auto errorView = new ErrorView(message);
        auto errorDialog = new ModalDialog(parentWindow, errorView, "Error");
        errorDialog->Run();
    }
}
