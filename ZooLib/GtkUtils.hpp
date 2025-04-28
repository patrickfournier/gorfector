#pragma once

#include <gtk/gtk.h>
#include <queue>

namespace ZooLib
{
    static GtkWidget *FindWidgetByName(GtkWidget *root, const char *name)
    {
        std::queue<GtkWidget *> widgetQueue;
        widgetQueue.push(root);

        while (!widgetQueue.empty())
        {
            auto current = widgetQueue.front();
            widgetQueue.pop();

            if (strcmp(gtk_widget_get_name(current), name) == 0)
            {
                return current;
            }

            for (auto child = gtk_widget_get_first_child(current); child != nullptr;
                 child = gtk_widget_get_next_sibling(child))
            {
                widgetQueue.push(child);
            }
        }

        return nullptr;
    }
}
