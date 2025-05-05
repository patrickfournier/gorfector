#pragma once

#include <gtk/gtk.h>
#include <queue>

namespace ZooLib
{
    /**
     * \brief Finds a widget by its name in the widget hierarchy starting from the given root.
     * \param root The root widget to start the search from.
     * \param name The name of the widget to find.
     * \return A pointer to the widget if found, otherwise nullptr.
     */
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

    /**
     * \brief Finds the first parent of the given widget that matches the specified GType.
     * \param widget The widget whose parent is to be found.
     * \param type The GType to match.
     * \return A pointer to the parent widget if found, otherwise nullptr.
     */
    static GtkWidget *GetParentOfType(GtkWidget *widget, GType type)
    {
        auto parent = gtk_widget_get_parent(widget);
        while (parent != nullptr && !G_TYPE_CHECK_INSTANCE_TYPE(parent, type))
        {
            parent = gtk_widget_get_parent(parent);
        }
        return parent;
    }
}
