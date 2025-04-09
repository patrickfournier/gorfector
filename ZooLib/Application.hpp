#pragma once

#include <gtk/gtk.h>
#include <string>
#include "ObserverManager.hpp"
#include "State.hpp"

namespace ZooLib
{
    class Application
    {
    protected:
        State m_State;
        ObserverManager m_ObserverManager;

        GtkApplication *m_GtkApp;
        GtkWindow *m_MainWindow{};

        [[nodiscard]] virtual GApplicationFlags GetApplicationFlags() = 0;

        [[nodiscard]] virtual std::string GetMainWindowTitle() = 0;

        [[nodiscard]] virtual std::tuple<int, int> GetMainWindowSize() = 0;

        virtual void PopulateMainWindow() = 0;

        void OnActivate(GtkApplication *app);

    public:
        Application();

        virtual void Initialize();

        virtual ~Application();

        int Run(int argc = 0, char **argv = nullptr) const;

        [[nodiscard]] virtual std::string GetApplicationId() const = 0;

        [[nodiscard]] GtkWindow *GetMainWindow() const
        {
            return m_MainWindow;
        }

        [[nodiscard]] State *GetState()
        {
            return &m_State;
        }

        [[nodiscard]] ObserverManager *GetObserverManager()
        {
            return &m_ObserverManager;
        }
    };
}
