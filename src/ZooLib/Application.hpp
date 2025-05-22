#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>
#include <string>

#include "CommandDispatcher.hpp"
#include "ObserverManager.hpp"
#include "PathUtils.hpp"
#include "SignalSupport.hpp"
#include "State.hpp"

#include "config.h"

namespace ZooLib
{
    class AppMenuBarBuilder;

    /**
     * \class Application
     * \brief Base class for the main application logic.
     *
     * The `Application` class serves as the foundation for building a GTK-based application.
     * It provides mechanisms for managing commands, application state, observers, and the main UI components.
     * Derived classes must implement several pure virtual methods to define application-specific behavior.
     */
    class Application
    {
    protected:
        /**
         * \brief Command dispatcher for handling application commands.
         *
         * The `CommandDispatcher` is used to register, unregister, and dispatch commands
         * to their respective handlers. It enables a modular and extensible command-handling mechanism.
         */
        CommandDispatcher m_Dispatcher{};

        /**
         * \brief Application state manager.
         *
         * The `State` object holds the current state of the application and provides
         * mechanisms for state management and persistence.
         */
        State m_State{};

        /**
         * \brief Observer manager for managing event listeners.
         *
         * The `ObserverManager` allows the application to register and notify observers
         * about specific events or changes in state.
         */
        ObserverManager m_ObserverManager{};

        /**
         * \brief Pointer to the GTK application instance.
         *
         * The `AdwApplication` object represents the main GTK application.
         */
        AdwApplication *m_GtkApp;

        /**
         * \brief Pointer to the main application window.
         *
         * The `GtkWidget` object represents the main window of the application.
         */
        GtkWidget *m_MainWindow{};

        /**
         * \brief Pointer to the menu button widget.
         *
         * The `GtkWidget` object represents the menu button in the application's UI.
         */
        GtkWidget *m_MenuButton{};

        /**
         * \brief Path to the temporary directory used by the application.
         *
         * This path is dynamically created and used for storing temporary files during the application's runtime.
         */
        std::filesystem::path m_TempDirectoryPath;

        std::filesystem::path m_UserConfigDirectoryPath;
        std::filesystem::path m_SystemConfigDirectoryPath;

        /**
         * \brief Indicates whether the application is running in test mode.
         *
         * When set to `true`, the application operates in a test environment,
         * playing back the actions in `m_TestActionsStack`.
         */
        bool m_TestMode{};

        /**
         * \brief Stack of test actions to be executed in test mode.
         *
         * The `m_TestActionsStack` is used to store
         * actions that are executed sequentially when the application is running in test mode.
         *
         * \remarks The stack is dynamically allocated and initialized only when test mode is enabled.
         * Actions can be pushed to the stack using the `PushTestAction` method.
         */
        std::vector<std::function<void(Application *)>> *m_TestActionsStack{};

        /**
         * \brief Constructs an `Application` instance.
         *
         * This constructor initializes the application and sets the test mode.
         * When `testMode` is set to `true`, the application operates in a test
         * environment, allowing for automated testing and playback of test actions.
         *
         * \param testMode A boolean indicating whether the application is in test mode.
         */
        explicit Application(bool testMode = false);

        /**
         * \brief Initializes the application.
         *
         * This method is called to perform any necessary setup before the application runs.
         */
        virtual void Initialize();

        /**
         * \brief Retrieves the application flags.
         *
         * This method must be implemented by derived classes to specify the flags
         * used to initialize the GTK application.
         *
         * \return The application flags as a `GApplicationFlags` value.
         */
        [[nodiscard]] virtual GApplicationFlags GetApplicationFlags() = 0;

        /**
         * \brief Retrieves the title of the main application window.
         *
         * This method must be implemented by derived classes to provide the title
         * displayed in the main application window.
         *
         * \return The main window title as a string.
         */
        [[nodiscard]] virtual std::string GetMainWindowTitle() = 0;

        /**
         * \brief Retrieves the default size of the main application window.
         *
         * This method must be implemented by derived classes to specify the width
         * and height of the main application window.
         *
         * \return A tuple containing the width and height of the main window.
         */
        [[nodiscard]] virtual std::tuple<int, int> GetMainWindowSize() = 0;

        /**
         * \brief Builds the main user interface of the application.
         *
         * This method must be implemented by derived classes to construct and return
         * the main UI widget of the application.
         *
         * \return A pointer to the root GTK widget of the application's UI.
         */
        virtual GtkWidget *BuildUI() = 0;

        /**
         * \brief Populates the application's menu bar.
         *
         * This method must be implemented by derived classes to define the structure
         * and content of the application's menu bar.
         *
         * \param menuBarBuilder A pointer to the `AppMenuBarBuilder` used to construct the menu bar.
         */
        virtual void PopulateMenuBar(AppMenuBarBuilder *menuBarBuilder) = 0;

        /**
         * \brief Called when the application is activated.
         *
         * This method is triggered when the application is launched or brought to the foreground.
         * Derived classes can override this method to define custom activation behavior.
         *
         * \param app A pointer to the `GtkApplication` instance.
         */
        virtual void OnActivate(GtkApplication *app);

        /**
         * \brief Dispatches a command for a given GTK action.
         *
         * This method is used to create and dispatch a command of type `TCommand`
         * when a GTK action is triggered. It retrieves the command instance and
         * passes it to the `CommandDispatcher` for execution.
         *
         * \tparam TCommand The type of the command to dispatch.
         * \param action A pointer to the `GSimpleAction` that triggered the command.
         * \param parameter A pointer to the `GVariant` parameter associated with the action.
         */
        template<typename TCommand>
        void SendCommandForAction(GSimpleAction *action, GVariant *parameter)
        {
            auto command = TCommand();
            m_Dispatcher.Dispatch<TCommand>(command);
        }

    public:
        /**
         * Classes deriving from `Application` should make their constructors protected or private and
         * implement a static `Create` method to ensure proper instantiation. The `Create` method
         * should call `Initialize` after creating the instance.
         *
         * ```cpp
         *  static DerivedApplication *Create(Arguments... args)
         *  {
         *      auto app = new DerivedApplication(args);
         *      app->Initialize();
         *      return app;
         *  }
         *  ```
         */

        Application() = delete;

        /**
         * \brief Destructor for the `Application` class.
         */
        virtual ~Application();

        /**
         * \brief Creates the menu bar for the application.
         *
         * This method is responsible for setting up the application's menu bar.
         */
        void CreateMenuBar();

        /**
         * \brief Binds a method to a GTK action.
         *
         * \tparam TClass The class that contains the method to bind.
         * \tparam TArgs The arguments of the method to bind.
         * \param actionName The name of the GTK action.
         * \param method The method to bind.
         * \param instance The instance of the class that contains the method.
         */
        template<typename TClass, typename... TArgs>
        void BindMethodToAction(const char *actionName, void (TClass::*method)(TArgs...), TClass *instance)
        {
            auto action = g_simple_action_new(actionName, nullptr);
            ConnectGtkSignal(instance, method, action, "activate");
            g_action_map_add_action(G_ACTION_MAP(m_GtkApp), G_ACTION(action));
        }

        /**
         * \brief Binds a method to a GTK action with a toggle state.
         *
         * \tparam TClass The class that contains the method to bind.
         * \tparam TArgs The arguments of the method to bind.
         * \param actionName The name of the GTK action.
         * \param method The method to bind.
         * \param instance The instance of the class that contains the method.
         * \param initialState The initial state of the action (true or false).
         *
         * \remarks The method should change the state of the action when called,
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

        /**
         * \brief Binds a command to a GTK action.
         *
         * \tparam TCommand The type of the command to bind.
         * \tparam TStateArgs Variadic template for additional state arguments.
         * \param actionName The name of the GTK action.
         * \param args Additional state arguments to bind to the command.
         */
        template<typename TCommand, typename... TStateArgs>
        void BindCommandToAction(const char *actionName, TStateArgs *...args)
        {
            m_Dispatcher.RegisterHandler<TCommand>(TCommand::Execute, args...);
            BindMethodToAction(actionName, &Application::SendCommandForAction<TCommand>, this);
        }

        /**
         * \brief Binds a command to a GTK action with a toggle state.
         *
         * \tparam TCommand The type of the command to bind.
         * \tparam TStateArgs Variadic template for additional state arguments.
         * \param actionName The name of the GTK action.
         * \param initialState The initial state of the action (true or false).
         * \param args Additional state arguments to bind to the command.
         */
        template<typename TCommand, typename... TStateArgs>
        void BindCommandToToggleAction(const char *actionName, bool initialState, TStateArgs *...args)
        {
            m_Dispatcher.RegisterHandler<TCommand>(TCommand::Execute, args...);
            BindMethodToToggleAction(actionName, &Application::SendCommandForAction<TCommand>, this, initialState);
        }

        /**
         * \brief Sets keyboard accelerators for a GTK action.
         *
         * \param actionName The name of the GTK action.
         * \param accelerators A vector of accelerator strings (e.g., "Ctrl+Q").
         */
        void SetAcceleratorForAction(const char *actionName, std::vector<const char *> &&accelerators) const
        {
            if (accelerators.back() != nullptr)
            {
                accelerators.push_back(nullptr);
            }
            gtk_application_set_accels_for_action(GTK_APPLICATION(m_GtkApp), actionName, accelerators.data());
        }

        /**
         * \brief Runs the application.
         *
         * \param argc The number of command-line arguments.
         * \param argv The array of command-line arguments.
         * \return The exit code of the application.
         */
        int Run(int argc = 0, char **argv = nullptr) const;

        /**
         * \brief Quits the application.
         *
         * \param action Optional GTK action that triggered the quit.
         * \param parameter Optional parameter for the action.
         */
        void Quit(GSimpleAction *action = nullptr, GVariant *parameter = nullptr)
        {
            g_application_quit(G_APPLICATION(m_GtkApp));
        }

        /**
         * \brief Gets the application ID (com.publisher_name.application_name).
         *
         * \return The application ID as a string.
         */
        [[nodiscard]] virtual std::string GetApplicationId() const
        {
            return APP_ID;
        }

        /**
         * \brief Gets the application name.
         *
         * \return The application name as a string.
         */
        [[nodiscard]] virtual std::string GetApplicationName() const = 0;

        /**
         * \brief Gets the main application window.
         *
         * \return A pointer to the main `AdwApplicationWindow`.
         */
        [[nodiscard]] AdwApplicationWindow *GetMainWindow() const
        {
            return ADW_APPLICATION_WINDOW(m_MainWindow);
        }

        /**
         * \brief Gets the application state manager.
         *
         * \return A pointer to the `State` object.
         */
        [[nodiscard]] State *GetState()
        {
            return &m_State;
        }

        /**
         * \brief Gets the observer manager.
         *
         * \return A pointer to the `ObserverManager` object.
         */
        [[nodiscard]] ObserverManager *GetObserverManager()
        {
            return &m_ObserverManager;
        }

        /**
         * \brief Gets the temporary directory path for the application.
         *
         * \return A reference to the temporary directory path.
         */
        [[nodiscard]] const std::filesystem::path &GetTemporaryDirectory()
        {
            if (m_TempDirectoryPath.empty())
            {
                auto xdgRuntimeDir =
                        std::filesystem::path(g_get_user_runtime_dir()) / "app" / (GetApplicationId() + "_XXXXXX");
                auto homeTmpStr = strdup(xdgRuntimeDir.c_str());
                m_TempDirectoryPath = std::filesystem::path(mkdtemp(homeTmpStr));
                free(homeTmpStr);
            }

            return m_TempDirectoryPath;
        }

        /**
         * \brief Gets the user-specific configuration directory path.
         *
         * This method retrieves the path to the user's configuration directory for this application.
         * The path is lazily initialized on first call and created if it doesn't exist.
         * It's typically located at ~/.config/[application-id]/ on Linux systems.
         *
         * \return A reference to the user configuration directory path.
         */
        [[nodiscard]] const std::filesystem::path &GetUserConfigDirectoryPath()
        {
            if (m_UserConfigDirectoryPath.empty())
            {
                m_UserConfigDirectoryPath = std::filesystem::path(g_get_user_config_dir()) / GetApplicationId();
                if (!std::filesystem::exists(m_UserConfigDirectoryPath))
                {
                    std::filesystem::create_directories(m_UserConfigDirectoryPath);
                }
            }

            return m_UserConfigDirectoryPath;
        }

        /**
         * \brief Gets the system-wide configuration directory path.
         *
         * This method retrieves the path to the system-wide configuration directory for this application.
         * The path is lazily initialized on first call.
         *
         * \return A reference to the system-wide configuration directory path.
         */
        [[nodiscard]] const std::filesystem::path &GetSystemConfigDirectoryPath()
        {
            if (m_SystemConfigDirectoryPath.empty())
            {
                m_SystemConfigDirectoryPath = RelocatePath(std::filesystem::path(DATADIR) / GetApplicationId());
            }

            return m_SystemConfigDirectoryPath;
        }

        /**
         * \brief Adds a test action to the execution stack.
         *
         * This method allows adding actions that will be executed sequentially
         * when the application is running in test mode. If the test actions stack
         * doesn't exist yet, it is automatically created.
         *
         * \param action A function to execute that takes a pointer to the application
         *               instance as a parameter.
         *
         * \remarks This method is primarily used for automated testing,
         *          allowing to simulate user interactions or specific behaviors
         *          during test execution.
         */
        void PushTestAction(const std::function<void(Application *)> &action)
        {
            if (m_TestActionsStack == nullptr)
            {
                m_TestActionsStack = new std::vector<std::function<void(Application *)>>();
            }

            m_TestActionsStack->push_back(action);
        }
    };
}
