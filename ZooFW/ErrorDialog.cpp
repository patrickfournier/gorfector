#include <gtk/gtk.h>

namespace Zoo
{
    static void CloseWindow(GtkWidget *widget, gpointer data)
    {
        gtk_window_destroy(GTK_WINDOW(data));
    }

    void ShowUserError(GtkWindow *parent, const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        char message[1024];
        vsnprintf(message, sizeof(message), format, args);
        va_end(args);

        auto window = gtk_window_new();
        gtk_window_set_transient_for(GTK_WINDOW(window), parent);
        gtk_window_set_modal(GTK_WINDOW(window), true);
        gtk_window_set_title(GTK_WINDOW(window), "Error");
        gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);

        auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_window_set_child(GTK_WINDOW(window), box);
        auto label = gtk_label_new(message);
        gtk_box_append(GTK_BOX(box), label);
        auto button = gtk_button_new_with_label("OK");
        gtk_box_append(GTK_BOX(box), button);
        g_signal_connect(button, "clicked", G_CALLBACK(CloseWindow), window);

        gtk_window_present(GTK_WINDOW(window));
    }
}
