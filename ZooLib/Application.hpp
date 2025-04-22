#pragma once


#include <adwaita.h>
#include <gtk/gtk.h>
#include <string>

#include "CommandDispatcher.hpp"
#include "ObserverManager.hpp"
#include "SignalSupport.hpp"
#include "State.hpp"

namespace ZooLib
{
    class AppMenuBarBuilder;
    class Application
    {
    protected:
        CommandDispatcher m_Dispatcher{};
        State m_State{};
        ObserverManager m_ObserverManager{};

        AdwApplication *m_GtkApp;
        GtkWidget *m_MainWindow{};
        GtkWidget *m_MenuButton{};

        std::filesystem::path m_TempDirectoryPath;

        [[nodiscard]] virtual GApplicationFlags GetApplicationFlags() = 0;

        [[nodiscard]] virtual std::string GetMainWindowTitle() = 0;

        [[nodiscard]] virtual std::tuple<int, int> GetMainWindowSize() = 0;

        virtual GtkWidget *CreateContent() = 0;
        virtual void PopulateMenuBar(AppMenuBarBuilder *menuBarBuilder) = 0;

        virtual void OnActivate(GtkApplication *app);

        template<typename TCommand>
        void SendCommandForAction(GSimpleAction *action, GVariant *parameter)
        {
            auto command = TCommand();
            m_Dispatcher.Dispatch<TCommand>(command);
        }

        void ToggleBooleanAction(GSimpleAction *action, GVariant *parameter)
        {
            auto state = g_variant_get_boolean(parameter);
            g_simple_action_set_state(action, g_variant_new_boolean(!state));
        }

    public:
        Application();

        virtual void Initialize();

        virtual ~Application();

        void CreateMenuBar();

        template<typename TClass, typename... TArgs>
        void BindMethodToAction(const char *actionName, void (TClass::*method)(TArgs...), TClass *instance)
        {
            auto action = g_simple_action_new(actionName, nullptr);
            ConnectGtkSignal(instance, method, action, "activate");
            g_action_map_add_action(G_ACTION_MAP(m_GtkApp), G_ACTION(action));
        }

        /**
         * Binds a method to a GTK action with a toggle state.
         *
         * @tparam TClass The class that contains the method to bind.
         * @tparam TArgs The arguments of the method to bind.
         * @param actionName The name of the GTK action.
         * @param method The method to bind.
         * @param instance The instance of the class that contains the method.
         * @param initialState The initial state of the action (true or false).
         *
         * @remarks The method should change the state of the action when called,
         * with `g_simple_action_set_state(action, g_variant_new_boolean(!state));`
         */
        template<typename TClass, typename... TArgs>
        void BindMethodToToggleAction(
                const char *actionName, void (TClass::*method)(TArgs...), TClass *instance, bool initialState)
        {
            auto variant = g_variant_new_boolean(initialState);
            auto action = g_simple_action_new_stateful(actionName, nullptr, variant);
            ConnectGtkSignal(instance, method, action, "activate");
            g_action_map_add_action(G_ACTION_MAP(m_GtkApp), G_ACTION(action));
        }

        template<typename TCommand, typename... TStateArgs>
        void BindCommandToAction(const char *actionName, TStateArgs *...args)
        {
            m_Dispatcher.RegisterHandler<TCommand>(TCommand::Execute, args...);
            BindMethodToAction(actionName, &Application::SendCommandForAction<TCommand>, this);
        }

        template<typename TCommand, typename... TStateArgs>
        void BindCommandToToggleAction(const char *actionName, bool initialState, TStateArgs *...args)
        {
            m_Dispatcher.RegisterHandler<TCommand>(TCommand::Execute, args...);
            BindMethodToToggleAction(actionName, &Application::SendCommandForAction<TCommand>, this, initialState);
        }

        void SetAcceleratorForAction(const char *actionName, std::vector<const char *> &&accelerators) const
        {
            if (accelerators.back() != nullptr)
            {
                accelerators.push_back(nullptr);
            }
            gtk_application_set_accels_for_action(GTK_APPLICATION(m_GtkApp), actionName, accelerators.data());
        }

        int Run(int argc = 0, char **argv = nullptr) const;

        void Quit(GSimpleAction *action = nullptr, GVariant *parameter = nullptr)
        {
            g_application_quit(G_APPLICATION(m_GtkApp));
        }

        [[nodiscard]] virtual std::string GetApplicationId() const = 0;

        [[nodiscard]] AdwApplicationWindow *GetMainWindow() const
        {
            return ADW_APPLICATION_WINDOW(m_MainWindow);
        }

        [[nodiscard]] State *GetState()
        {
            return &m_State;
        }

        [[nodiscard]] ObserverManager *GetObserverManager()
        {
            return &m_ObserverManager;
        }

        [[nodiscard]] const std::filesystem::path &GetTemporaryDirectory()
        {
            if (m_TempDirectoryPath.empty())
            {
                auto xdgRuntimeDir = std::filesystem::path(g_get_user_runtime_dir()) / "zooscan_XXXXXX";
                auto homeTmpStr = strdup(xdgRuntimeDir.c_str());
                m_TempDirectoryPath = std::filesystem::path(mkdtemp(homeTmpStr));
                free(homeTmpStr);
            }

            return m_TempDirectoryPath;
        }
    };
}
