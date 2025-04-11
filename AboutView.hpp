#pragma once

#include <gtk/gtk.h>

#include "ZooLib/View.hpp"

namespace ZooScan
{

    class AboutView : public ZooLib::View
    {
        GtkWidget *m_RootWidget;

    public:
        AboutView()
        {
            m_RootWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            auto label = gtk_label_new("ZooScan 1.0");
            gtk_box_append(GTK_BOX(m_RootWidget), label);
            label = gtk_label_new("ZooScan is a software for scanning images.");
            gtk_box_append(GTK_BOX(m_RootWidget), label);
            label = gtk_label_new("Author: Patrick Fournier");
            gtk_box_append(GTK_BOX(m_RootWidget), label);
            label = gtk_label_new("License: GPLv3");
            gtk_box_append(GTK_BOX(m_RootWidget), label);
        }

        GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        void Update(uint64_t lastSeenVersion) override
        {
            // No update needed for the about dialog
        }
    };

}
