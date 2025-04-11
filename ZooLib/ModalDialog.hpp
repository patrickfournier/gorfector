#pragma once

#include <gtk/gtk.h>

#include "View.hpp"


namespace ZooLib
{

    class ModalDialog
    {
        GtkWidget *m_Dialog;
        View *m_View;

        static void OnClose(GtkWidget *widget, gpointer data)
        {
            auto *dialog = static_cast<ModalDialog *>(data);
            delete dialog;
        }

    public:
        ModalDialog(GtkWindow *parentWindow, View *content, const char *title)
        {
            m_View = content;

            m_Dialog = gtk_dialog_new();
            gtk_window_set_title(GTK_WINDOW(m_Dialog), title);
            gtk_window_set_resizable(GTK_WINDOW(m_Dialog), false);
            gtk_window_set_modal(GTK_WINDOW(m_Dialog), true);
            gtk_window_set_transient_for(GTK_WINDOW(m_Dialog), parentWindow);

            gtk_box_append(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(m_Dialog))), content->GetRootWidget());

            g_signal_connect(m_Dialog, "close-request", G_CALLBACK(ModalDialog::OnClose), this);
        }

        virtual ~ModalDialog()
        {
            delete m_View;
            m_View = nullptr;
        }

        void Run()
        {
            gtk_window_present(GTK_WINDOW(m_Dialog));
        }
    };
}
