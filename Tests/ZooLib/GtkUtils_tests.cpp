#include "ZooLib/GtkUtils.hpp"
#include <gtk/gtk.h>
#include "gtest/gtest.h"

namespace ZooLib
{
    TEST(ZooLib_GtkUtilsTest, FindNamedChildrenWorksOnSimpleHierarchy)
    {
        auto *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        auto *child1 = gtk_label_new("Child 1");
        auto *child2 = gtk_label_new("Child 2");
        auto *child3 = gtk_label_new("Child 3");
        gtk_widget_set_name(child1, "child1");
        gtk_widget_set_name(child2, "child2");
        gtk_widget_set_name(child3, "child3");
        gtk_box_append(GTK_BOX(root), child1);
        gtk_box_append(GTK_BOX(root), child2);
        gtk_box_append(GTK_BOX(root), child3);

        auto findResult = FindWidgetByName(root, "child2");
        EXPECT_EQ(findResult, child2);

        findResult = FindWidgetByName(root, "notfound");
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, FindNamedChildrenWorksOnRoot)
    {
        auto *root = gtk_label_new("Child 1");
        gtk_widget_set_name(root, "child1");

        auto findResult = FindWidgetByName(root, "child1");
        EXPECT_EQ(findResult, root);

        findResult = FindWidgetByName(root, "notfound");
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, FindNamedChildrenWorksOnNullRoot)
    {
        auto findResult = FindWidgetByName(nullptr, "child1");
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, FindNamedChildrenWorksOnComplexHierarchy)
    {
        GtkWidget *importantChild1, *importantChild2, *importantChild3;

        auto *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        auto *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(root), box1);
        box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(root), box1);

        auto box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        importantChild1 = box2;
        gtk_widget_set_name(box2, "importantChild1");
        gtk_box_append(GTK_BOX(box1), box2);
        box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(box1), box2);
        box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(box1), box2);

        auto *child1 = gtk_label_new("Label 1");
        gtk_box_append(GTK_BOX(box2), child1);
        child1 = gtk_label_new("Label 2");
        gtk_box_append(GTK_BOX(box2), child1);
        child1 = gtk_label_new("Label 3");
        gtk_box_append(GTK_BOX(box2), child1);
        importantChild2 = child1;
        gtk_widget_set_name(child1, "importantChild2");

        box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(root), box1);

        auto *child2 = gtk_label_new("Label 4");
        importantChild3 = child2;
        gtk_widget_set_name(child2, "importantChild3");
        gtk_box_append(GTK_BOX(box1), child2);
        child2 = gtk_label_new("Label 5");
        gtk_box_append(GTK_BOX(box1), child2);
        child2 = gtk_label_new("Label 6");
        gtk_box_append(GTK_BOX(box1), child2);

        auto findResult = FindWidgetByName(root, "importantChild1");
        EXPECT_EQ(findResult, importantChild1);

        findResult = FindWidgetByName(root, "importantChild2");
        EXPECT_EQ(findResult, importantChild2);

        findResult = FindWidgetByName(root, "importantChild3");
        EXPECT_EQ(findResult, importantChild3);

        findResult = FindWidgetByName(root, "notfound");
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, GetParentOfTypeWorksOnNullRoot)
    {
        auto findResult = GetParentOfType(nullptr, GTK_TYPE_BOX);
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, GetParentOfTypeDoesNotConsiderRoot)
    {
        auto *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        auto findResult = GetParentOfType(root, GTK_TYPE_BOX);
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, GetParentOfTypeWorksWithInvalidType)
    {
        auto *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        auto findResult = GetParentOfType(root, G_TYPE_INVALID);
        EXPECT_EQ(findResult, nullptr);
    }

    TEST(ZooLib_GtkUtilsTest, GetParentOfTypeReturnsFirstParent)
    {
        auto *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        auto child1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(root), child1);
        auto child2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(child1), child2);
        auto child3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(child2), child3);

        auto findResult = GetParentOfType(child2, GTK_TYPE_BOX);
        EXPECT_EQ(findResult, child1);
    }
}
