#pragma once

#include <gio/gio.h>
#include <string>
#include <vector>

namespace ZooLib
{
    /**
     * \class AppMenuBarBuilder
     * \brief A utility class for constructing a GTK menu bar using the GMenu API.
     *
     * This class provides a fluent interface for building hierarchical menu structures,
     * including submenus, sections, and individual menu items. It manages the internal
     * stack of menus to allow nested menu creation.
     */
    class AppMenuBarBuilder
    {
        /**
         * \brief The root menu model for the menu bar.
         */
        GMenu *m_MenuBarModel{};

        /**
         * \brief A stack to manage the current menu context during nested menu creation.
         */
        std::vector<GMenu *> m_CurrentMenuStack{};

        /**
         * \brief Retrieves the current menu from the stack.
         * \return A pointer to the current GMenu.
         */
        [[nodiscard]] GMenu *GetCurrentMenu() const
        {
            return m_CurrentMenuStack.back();
        }

    public:
        /**
         * \brief Constructs a new AppMenuBarBuilder and initializes the root menu bar model.
         */
        AppMenuBarBuilder()
        {
            m_MenuBarModel = g_menu_new();
            m_CurrentMenuStack.push_back(m_MenuBarModel);
        }

        /**
         * \brief Begins a new submenu under the current menu.
         * \param name The name of the submenu.
         * \param position The position to insert the submenu (default is -1, appends to the end).
         * \return A pointer to the current AppMenuBarBuilder instance for chaining.
         */
        AppMenuBarBuilder *BeginSubMenu(const std::string &name, int position = -1)
        {
            auto *subMenu = g_menu_new();

            if (position >= 0)
            {
                g_menu_insert_submenu(GetCurrentMenu(), position, name.c_str(), G_MENU_MODEL(subMenu));
            }
            else
            {
                g_menu_append_submenu(GetCurrentMenu(), name.c_str(), G_MENU_MODEL(subMenu));
            }
            m_CurrentMenuStack.push_back(subMenu);
            return this;
        }

        /**
         * \brief Ends the current submenu and returns to the parent menu.
         * \return A pointer to the current AppMenuBarBuilder instance for chaining.
         */
        AppMenuBarBuilder *EndSubMenu()
        {
            m_CurrentMenuStack.pop_back();
            return this;
        }

        /**
         * \brief Adds a menu item to the current menu.
         * \param name The name of the menu item.
         * \param action The action associated with the menu item.
         * \param position The position to insert the menu item (default is -1, appends to the end).
         * \return A pointer to the current AppMenuBarBuilder instance for chaining.
         */
        AppMenuBarBuilder *AddMenuItem(const std::string &name, const std::string &action, int position = -1)
        {
            if (position >= 0)
            {
                g_menu_insert(GetCurrentMenu(), position, name.c_str(), action.c_str());
                return this;
            }

            g_menu_append(GetCurrentMenu(), name.c_str(), action.c_str());
            return this;
        }

        /**
         * \brief Begins a new section in the current menu.
         * \param position The position to insert the section (default is -1, appends to the end).
         * \return A pointer to the current AppMenuBarBuilder instance for chaining.
         */
        AppMenuBarBuilder *BeginSection(int position = -1)
        {
            auto *sectionMenu = g_menu_new();
            if (position >= 0)
            {
                g_menu_insert_section(GetCurrentMenu(), position, nullptr, G_MENU_MODEL(sectionMenu));
            }
            else
            {
                g_menu_append_section(GetCurrentMenu(), nullptr, G_MENU_MODEL(sectionMenu));
            }
            m_CurrentMenuStack.push_back(sectionMenu);
            return this;
        }

        /**
         * \brief Ends the current section and returns to the parent menu.
         * \return A pointer to the current AppMenuBarBuilder instance for chaining.
         */
        AppMenuBarBuilder *EndSection()
        {
            m_CurrentMenuStack.pop_back();
            return this;
        }

        /**
         * \brief Retrieves the root menu bar model.
         * \return A pointer to the GMenu representing the menu bar.
         */
        [[nodiscard]] GMenu *GetMenuBarModel() const
        {
            return m_MenuBarModel;
        }
    };
}
