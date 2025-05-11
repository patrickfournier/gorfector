#include <gtk/gtk.h>

#include "ZooLib/View.hpp"
#include "gtest/gtest.h"

namespace ZooLib
{
    class TestView : public View
    {
    public:
        static std::vector<const TestView *> s_DeletedViews;

    private:
        GtkWidget *m_RootWidget{nullptr};

        TestView()
        {
            m_RootWidget = gtk_label_new("Test");
        }

    public:
        static TestView *Create()
        {
            auto view = new TestView();
            view->PostCreateView();
            return view;
        }

        ~TestView() override
        {
            s_DeletedViews.push_back(this);
        }

        GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        void Update(const std::vector<uint64_t> &lastSeenVersions) override
        {
        }
    };

    std::vector<const TestView *> TestView::s_DeletedViews = {};

    TEST(ZooLib_ViewTests, UnreffingGtkWidgetDeletesView)
    {
        EXPECT_EQ(TestView::s_DeletedViews.size(), 0);

        auto window = gtk_window_new();

        auto *view = TestView::Create();
        auto rootWidget = view->GetRootWidget();
        EXPECT_NE(rootWidget, nullptr);
        gtk_window_set_child(GTK_WINDOW(window), rootWidget);
        gtk_window_destroy(GTK_WINDOW(window));

        EXPECT_EQ(TestView::s_DeletedViews.size(), 1);
        EXPECT_EQ(TestView::s_DeletedViews[0], view);
    }
}
