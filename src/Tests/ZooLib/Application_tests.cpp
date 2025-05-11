#include "gtest/gtest.h"

extern "C" {
#include <xdo.h>
}

#include "ZooLib/Application.hpp"

#include "Tests/TestsSupport/Applications.hpp"
#include "Tests/TestsSupport/Commands.hpp"

namespace ZooLib
{
    class ExtendedTestApplication : public TestsSupport::TestApplication
    {
        int m_CallCount{};

        using TestApplication::TestApplication;

    public:
        static ExtendedTestApplication *Create(bool testMode = false)
        {
            auto app = new ExtendedTestApplication(testMode);
            app->Initialize();
            return app;
        }

        std::string GetApplicationId() const override
        {
            return Application::GetApplicationId() + "_extended_tests";
        }

        AdwApplication *GetGtkApp() const
        {
            return m_GtkApp;
        }

        void IncrementCallCount(GSimpleAction *action, GVariant *parameter)
        {
            ++m_CallCount;
        }

        int GetCallCount() const
        {
            return m_CallCount;
        }
    };

    TEST(ZooLib_ApplicationTests, CanCreateApplication)
    {
        auto app = TestsSupport::TestApplication::Create();
        EXPECT_NE(app, nullptr);
        delete app;
    }

    TEST(ZooLib_ApplicationTests, BindMethodToActionWorks)
    {
        auto app = ExtendedTestApplication::Create();
        EXPECT_EQ(app->GetCallCount(), 0);

        app->BindMethodToAction("app.open", &ExtendedTestApplication::IncrementCallCount, app);

        auto action = g_action_map_lookup_action(G_ACTION_MAP(app->GetGtkApp()), "app.open");
        EXPECT_NE(action, nullptr);
        g_action_activate(action, nullptr);
        EXPECT_EQ(app->GetCallCount(), 1);

        delete app;
    }

    TEST(ZooLib_ApplicationTests, BindMethodToToggleActionWorks)
    {
        auto app = ExtendedTestApplication::Create();
        EXPECT_EQ(app->GetCallCount(), 0);

        app->BindMethodToToggleAction("app.toggle", &ExtendedTestApplication::IncrementCallCount, app, false);

        auto action = g_action_map_lookup_action(G_ACTION_MAP(app->GetGtkApp()), "app.toggle");
        EXPECT_NE(action, nullptr);
        g_action_activate(action, nullptr);
        EXPECT_EQ(app->GetCallCount(), 1);

        delete app;
    }

    TEST(ZooLib_ApplicationTests, BindCommandToActionWorks)
    {
        auto app = ExtendedTestApplication::Create();
        TestsSupport::CommandA::Reset();
        EXPECT_EQ(TestsSupport::CommandA::GetCommandHandled(), 0);

        app->BindCommandToAction<TestsSupport::CommandA>("app.open");

        auto action = g_action_map_lookup_action(G_ACTION_MAP(app->GetGtkApp()), "app.open");
        EXPECT_NE(action, nullptr);
        g_action_activate(action, nullptr);
        EXPECT_EQ(TestsSupport::CommandA::GetCommandHandled(), 1);

        delete app;
    }

    TEST(ZooLib_ApplicationTests, BindCommandToToggleActionWorks)
    {
        auto app = ExtendedTestApplication::Create();
        TestsSupport::CommandA::Reset();
        EXPECT_EQ(TestsSupport::CommandA::GetCommandHandled(), 0);

        app->BindCommandToToggleAction<TestsSupport::CommandA>("app.toggle", false);

        auto action = g_action_map_lookup_action(G_ACTION_MAP(app->GetGtkApp()), "app.toggle");
        EXPECT_NE(action, nullptr);
        g_action_activate(action, nullptr);
        EXPECT_EQ(TestsSupport::CommandA::GetCommandHandled(), 1);

        delete app;
    }

    TEST(ZooLib_ApplicationTests, SettingShortcutWorks)
    {
        auto app = ExtendedTestApplication::Create(true);
        EXPECT_EQ(app->GetCallCount(), 0);

        app->BindMethodToAction<ExtendedTestApplication>("open", &ExtendedTestApplication::IncrementCallCount, app);
        app->SetAcceleratorForAction("app.open", {"<Shift>O"});

        auto action = g_action_map_lookup_action(G_ACTION_MAP(app->GetGtkApp()), "open");
        EXPECT_NE(action, nullptr);

        app->PushTestAction([](Application *) {
            xdo_t *x = xdo_new(nullptr);
            xdo_send_keysequence_window(x, CURRENTWINDOW, "Shift_L+O", 0);
            xdo_free(x);
        });

        app->Run();

        EXPECT_EQ(app->GetCallCount(), 1);

        delete app;
    }

    TEST(ZooLib_ApplicationTests, CanCreateFileInTempDir)
    {
        auto app = TestsSupport::TestApplication::Create();

        auto tempDir = app->GetTemporaryDirectory();
        EXPECT_FALSE(tempDir.empty());

        auto filePath = tempDir / "testfile.txt";
        std::ofstream file(filePath);
        EXPECT_TRUE(file.is_open());
        file << "Hello, World!";
        file.close();
        EXPECT_TRUE(std::filesystem::exists(filePath));
        std::filesystem::remove(filePath);
        EXPECT_FALSE(std::filesystem::exists(filePath));

        delete app;
    }
}
