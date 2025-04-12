#pragma once

#include <gtk/gtk.h>

#include "View.hpp"


namespace ZooLib
{

    class DialogWindow
    {
    public:
        enum class Flags : uint32_t
        {
            Resizable = 1 << 0,
            Modal = 1 << 1,
        };

        constexpr static uint32_t k_DefaultFlags = 0;
        static bool HasFlag(uint32_t flagSet, Flags flag)
        {
            return (flagSet & static_cast<uint32_t>(flag)) == static_cast<uint32_t>(flag);
        }

    private:
        GtkWidget *m_Dialog;
        View *m_View;

        static void OnClose(GtkWidget *widget, gpointer data)
        {
            auto *dialog = static_cast<DialogWindow *>(data);
            delete dialog;
        }

    public:
        DialogWindow(GtkWindow *parentWindow, View *content, const char *title, Flags flag)
            : DialogWindow(parentWindow, content, title, static_cast<uint32_t>(flag))
        {
        }

        DialogWindow(GtkWindow *parentWindow, View *content, const char *title, uint32_t flags = k_DefaultFlags)
        {
            m_View = content;

            m_Dialog = gtk_dialog_new();
            gtk_window_set_title(GTK_WINDOW(m_Dialog), title);
            if (HasFlag(flags, Flags::Resizable))
            {
                gtk_window_set_resizable(GTK_WINDOW(m_Dialog), true);
            }
            else
            {
                gtk_window_set_resizable(GTK_WINDOW(m_Dialog), false);
            }
            if (HasFlag(flags, Flags::Modal))
            {
                gtk_window_set_modal(GTK_WINDOW(m_Dialog), true);
            }
            else
            {
                gtk_window_set_modal(GTK_WINDOW(m_Dialog), false);
            }
            gtk_window_set_transient_for(GTK_WINDOW(m_Dialog), parentWindow);

            gtk_box_append(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(m_Dialog))), content->GetRootWidget());

            g_signal_connect(m_Dialog, "close-request", G_CALLBACK(DialogWindow::OnClose), this);
        }

        virtual ~DialogWindow()
        {
            delete m_View;
            m_View = nullptr;
        }

        GtkWidget *GetGtkWidget()
        {
            return m_Dialog;
        }

        void Run()
        {
            gtk_window_present(GTK_WINDOW(m_Dialog));
        }
    };
}
