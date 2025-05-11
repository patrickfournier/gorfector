#pragma once

#include "ZooLib/AppMenuBarBuilder.hpp"
#include "ZooLib/Application.hpp"

namespace TestsSupport
{
    class TestApplication : public ZooLib::Application
    {
    protected:
        using Application::Application;

        GApplicationFlags GetApplicationFlags() override
        {
            return G_APPLICATION_DEFAULT_FLAGS;
        }

        std::string GetMainWindowTitle() override
        {
            return "Test Application";
        }

        std::tuple<int, int> GetMainWindowSize() override
        {
            return {800, 600};
        }

        GtkWidget *BuildUI() override
        {
            auto label = gtk_label_new("Test Application");
            return label;
        }

        void PopulateMenuBar(ZooLib::AppMenuBarBuilder *menuBarBuilder) override
        {
            menuBarBuilder->BeginSection()
                    ->AddMenuItem("Open...", "app.open")
                    ->EndSection()
                    ->BeginSection()
                    ->AddMenuItem("Save...", "app.save")
                    ->EndSection()
                    ->BeginSection()
                    ->AddMenuItem("Settings...", "app.preferences")
                    ->AddMenuItem("About...", "app.about")
                    ->EndSection();
        }

    public:
        static TestApplication *Create(bool testMode = false)
        {
            auto app = new TestApplication(testMode);
            app->Initialize();
            return app;
        }

        std::string GetApplicationId() const override
        {
            return Application::GetApplicationId() + "_tests";
        }

        std::string GetApplicationName() const override
        {
            return "Test Application";
        }
    };
}
