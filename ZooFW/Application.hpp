#pragma once

#include "State.hpp"
#include "ObserverManager.hpp"
#include <string>
#include <gtk/gtk.h>

namespace Zoo
{
    class Application
    {
    protected:
        State m_State;
        ObserverManager m_ObserverManager;

        GtkApplication *m_GtkApp;
        GtkWindow *m_MainWindow{};

        virtual std::string GetApplicationId() = 0;

        virtual GApplicationFlags GetApplicationFlags() = 0;

        virtual std::string GetMainWindowTitle() = 0;

        virtual std::tuple<int, int> GetMainWindowSize() = 0;

        virtual void PopulateMainWindow() = 0;

        void OnActivate(GtkApplication *app);

    public:
        Application();

        virtual void Initialize();

        virtual ~Application();

        int Run(int argc = 0, char **argv = nullptr) const;

        GtkWindow *GetMainWindow()
        { return m_MainWindow; }

        State *GetState()
        { return &m_State; }

        ObserverManager *GetObserverManager()
        { return &m_ObserverManager; }
    };
}
