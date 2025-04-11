#pragma once
#include <gio/gio.h>
#include <string>
#include <vector>

namespace ZooLib
{
    class AppMenuBarBuilder
    {
        GMenu *m_MenuBarModel{};
        std::vector<GMenu *> m_CurrentMenuStack{};

        GMenu *CurrentMenu()
        {
            return m_CurrentMenuStack.back();
        }

    public:
        AppMenuBarBuilder()
        {
            m_MenuBarModel = g_menu_new();
            m_CurrentMenuStack.push_back(m_MenuBarModel);
        }

        AppMenuBarBuilder *BeginSubMenu(const std::string &name, int position = -1)
        {
            auto *subMenu = g_menu_new();

            if (position >= 0)
            {
                g_menu_insert_submenu(CurrentMenu(), position, name.c_str(), G_MENU_MODEL(subMenu));
            }
            else
            {
                g_menu_append_submenu(CurrentMenu(), name.c_str(), G_MENU_MODEL(subMenu));
            }
            m_CurrentMenuStack.push_back(subMenu);
            return this;
        }

        AppMenuBarBuilder *EndSubMenu()
        {
            m_CurrentMenuStack.pop_back();
            return this;
        }

        AppMenuBarBuilder *AddMenuItem(const std::string &name, const std::string &action, int position = -1)
        {
            if (position >= 0)
            {
                g_menu_insert(CurrentMenu(), position, name.c_str(), action.c_str());
                return this;
            }

            g_menu_append(CurrentMenu(), name.c_str(), action.c_str());
            return this;
        }

        AppMenuBarBuilder *BeginSection(int position = -1)
        {
            auto *sectionMenu = g_menu_new();
            if (position >= 0)
            {
                g_menu_insert_section(CurrentMenu(), position, nullptr, G_MENU_MODEL(sectionMenu));
            }
            else
            {
                g_menu_append_section(CurrentMenu(), nullptr, G_MENU_MODEL(sectionMenu));
            }
            m_CurrentMenuStack.push_back(sectionMenu);
            return this;
        }

        AppMenuBarBuilder *EndSection()
        {
            m_CurrentMenuStack.pop_back();
            return this;
        }

        GMenu *GetMenuBarModel()
        {
            return m_MenuBarModel;
        }
    };
}
