#include <gtk/gtk.h>

#include "gtest/gtest.h"

#include "ZooLib/AppMenuBarBuilder.hpp"

namespace ZooLib
{
    std::string GetItemLabel(GMenu *menuModel, int index)
    {
        if (index < g_menu_model_get_n_items(G_MENU_MODEL(menuModel)))
        {
            auto attrIter = g_menu_model_iterate_item_attributes(G_MENU_MODEL(menuModel), index);
            auto stop = !g_menu_attribute_iter_next(attrIter);
            while (!stop)
            {
                auto attrName = g_menu_attribute_iter_get_name(attrIter);
                if (attrName != nullptr && strcmp(attrName, "label") == 0)
                {
                    auto attrValue = g_menu_attribute_iter_get_value(attrIter);
                    auto label = g_variant_get_string(attrValue, nullptr);
                    g_variant_unref(attrValue);
                    g_object_unref(attrIter);
                    return std::string(label);
                }

                stop = !g_menu_attribute_iter_next(attrIter);
            }

            g_object_unref(attrIter);
        }

        return "";
    }

    TEST(ZooLib_AppMenuBarBuilderTests, AddMenuItemsWork)
    {
        AppMenuBarBuilder builder;
        builder.AddMenuItem("Edit", "action1")
                ->AddMenuItem("View", "action1")
                ->AddMenuItem("File", "action1", -1)
                ->AddMenuItem("Help", "action1", 4)
                ->AddMenuItem("Settings", "action1", 4)
                ->AddMenuItem("Tools", "action1", 2);

        auto menuBarModel = builder.GetMenuBarModel();
        auto menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(menuBarModel));
        EXPECT_EQ(menuItemCount, 6);

        EXPECT_EQ(GetItemLabel(menuBarModel, 0), "File");
        EXPECT_EQ(GetItemLabel(menuBarModel, 1), "Edit");
        EXPECT_EQ(GetItemLabel(menuBarModel, 2), "Tools");
        EXPECT_EQ(GetItemLabel(menuBarModel, 3), "View");
        EXPECT_EQ(GetItemLabel(menuBarModel, 4), "Help");
        EXPECT_EQ(GetItemLabel(menuBarModel, 5), "Settings");
    }

    TEST(ZooLib_AppMenuBarBuilderTests, AddSubMenuWork)
    {
        AppMenuBarBuilder builder;
        builder.AddMenuItem("File", "action1")
                ->AddMenuItem("Edit", "action1")
                ->AddMenuItem("View", "action1")

                ->BeginSubMenu("SubMenu1", -1)
                ->AddMenuItem("SubMenu1Item1", "action1")
                ->EndSubMenu()

                ->BeginSubMenu("SubMenu2")
                ->AddMenuItem("SubMenu2Item1", "action1")
                ->EndSubMenu()

                ->BeginSubMenu("SubMenu3", 2)
                ->AddMenuItem("SubMenu3Item1", "action1")
                ->EndSubMenu();

        auto menuBarModel = builder.GetMenuBarModel();
        auto menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(menuBarModel));
        EXPECT_EQ(menuItemCount, 6);

        EXPECT_EQ(GetItemLabel(menuBarModel, 0), "SubMenu1");
        EXPECT_EQ(GetItemLabel(menuBarModel, 1), "File");
        EXPECT_EQ(GetItemLabel(menuBarModel, 2), "SubMenu3");
        EXPECT_EQ(GetItemLabel(menuBarModel, 3), "Edit");
        EXPECT_EQ(GetItemLabel(menuBarModel, 4), "View");
        EXPECT_EQ(GetItemLabel(menuBarModel, 5), "SubMenu2");
    }

    TEST(ZooLib_AppMenuBarBuilderTests, AddSectionWorks)
    {
        AppMenuBarBuilder builder;
        builder.BeginSubMenu("File");
        auto menu = builder.GetCurrentMenu();

        builder.AddMenuItem("New", "action1")->AddMenuItem("Open", "action1")->BeginSection();
        auto section = builder.GetCurrentMenu();
        builder.AddMenuItem("Save", "action1")
                ->AddMenuItem("Save As", "action1")
                ->EndSection()
                ->AddMenuItem("Close", "action1")
                ->EndSubMenu();

        auto menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(menu));
        EXPECT_EQ(menuItemCount, 4);
        EXPECT_EQ(GetItemLabel(menu, 0), "New");
        EXPECT_EQ(GetItemLabel(menu, 1), "Open");
        EXPECT_EQ(GetItemLabel(menu, 2), "");
        EXPECT_EQ(GetItemLabel(menu, 3), "Close");

        menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(section));
        EXPECT_EQ(menuItemCount, 2);
        EXPECT_EQ(GetItemLabel(section, 0), "Save");
        EXPECT_EQ(GetItemLabel(section, 1), "Save As");
    }

    TEST(ZooLib_AppMenuBarBuilderTests, AddSectionWithPositionWorks)
    {
        AppMenuBarBuilder builder;
        builder.BeginSubMenu("File");
        auto menu = builder.GetCurrentMenu();

        builder.AddMenuItem("New", "action1")->AddMenuItem("Open", "action1")->BeginSection(0);
        auto section = builder.GetCurrentMenu();
        builder.AddMenuItem("Save", "action1")
                ->AddMenuItem("Save As", "action1")
                ->EndSection()
                ->AddMenuItem("Close", "action1")
                ->EndSubMenu();

        auto menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(menu));
        EXPECT_EQ(menuItemCount, 4);
        EXPECT_EQ(GetItemLabel(menu, 0), "");
        EXPECT_EQ(GetItemLabel(menu, 1), "New");
        EXPECT_EQ(GetItemLabel(menu, 2), "Open");
        EXPECT_EQ(GetItemLabel(menu, 3), "Close");

        menuItemCount = g_menu_model_get_n_items(G_MENU_MODEL(section));
        EXPECT_EQ(menuItemCount, 2);
        EXPECT_EQ(GetItemLabel(section, 0), "Save");
        EXPECT_EQ(GetItemLabel(section, 1), "Save As");
    }
}
