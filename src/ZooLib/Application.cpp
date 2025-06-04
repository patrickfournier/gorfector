#include <adwaita.h>

#include "AppMenuBarBuilder.hpp"
#include "Application.hpp"
#include "SignalSupport.hpp"

ZooLib::Application::Application(const bool testMode)
    : m_GtkApp(nullptr)
    , m_TestMode(testMode)
{
}

void ZooLib::Application::Initialize()
{
    m_GtkApp = adw_application_new(GetApplicationId().c_str(), GetApplicationFlags());
    ConnectGtkSignal(this, &Application::OnActivate, m_GtkApp, "activate");

    if (m_TestMode)
    {
        m_TestActionsStack = new std::vector<std::function<void(Application *)>>();
        m_TestActionsStack->emplace_back([](Application *appParam) { appParam->Quit(); });
    }
}

ZooLib::Application::~Application()
{

    if (m_ExecuteTestActionCallbackId != 0)
    {
        gtk_widget_remove_tick_callback(m_MainWindow, m_RunObserversCallbackId);
    }
    if (m_ExecuteTestActionCallbackId != 0)
    {
        gtk_widget_remove_tick_callback(m_MainWindow, m_ExecuteTestActionCallbackId);
    }

    g_object_unref(m_GtkApp);
    m_GtkApp = nullptr;
    delete m_TestActionsStack;
    m_TestActionsStack = nullptr;
}

void ZooLib::Application::CreateMenuBar()
{
    auto menuBar = new AppMenuBarBuilder();
    PopulateMenuBar(menuBar);
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(m_MenuButton), G_MENU_MODEL(menuBar->GetMenuBarModel()));
}

int ZooLib::Application::Run(int argc, char **argv) const
{
    return g_application_run(G_APPLICATION(m_GtkApp), argc, argv);
}

void ZooLib::Application::OnActivate(GtkApplication *app)
{
    m_MainWindow = adw_application_window_new(app);
    auto mainView = adw_toolbar_view_new();
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(m_MainWindow), mainView);

    auto header = adw_header_bar_new();
    auto titleLabel = adw_window_title_new(GetMainWindowTitle().c_str(), nullptr);
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(header), GTK_WIDGET(titleLabel));
    m_MenuButton = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(m_MenuButton), "open-menu-symbolic");
    adw_header_bar_pack_end(ADW_HEADER_BAR(header), m_MenuButton);
    adw_header_bar_set_decoration_layout(ADW_HEADER_BAR(header), "menu:minimize,maximize,close");
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(mainView), header);

    auto size = GetMainWindowSize();
    gtk_window_set_default_size(GTK_WINDOW(m_MainWindow), std::get<0>(size), std::get<1>(size));

    CreateMenuBar();
    auto content = BuildUI();
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(mainView), content);

    gtk_window_present(GTK_WINDOW(m_MainWindow));
    m_RunObserversCallbackId = gtk_widget_add_tick_callback(
            m_MainWindow,
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *localApp = static_cast<Application *>(data);
                localApp->m_ObserverManager.NotifyObservers();
                localApp->PurgeChangesets();
                return G_SOURCE_CONTINUE;
            },
            this, nullptr);

    if (m_TestMode)
    {
        m_ExecuteTestActionCallbackId = gtk_widget_add_tick_callback(
                m_MainWindow,
                [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                    auto app = static_cast<Application *>(data);

                    if (app->m_TestActionsStack != nullptr && !app->m_TestActionsStack->empty())
                    {
                        auto action = app->m_TestActionsStack->back();
                        app->m_TestActionsStack->pop_back();
                        action(app);
                        return G_SOURCE_CONTINUE;
                    }

                    return G_SOURCE_REMOVE;
                },
                this, nullptr);
    }
}

void ZooLib::Application::CreateDesktopEntryForAppImage(const char *wmClass)
{
    auto mountPoint = getenv("APPDIR");
    auto xdgDataHome = getenv("XDG_DATA_HOME");
    std::filesystem::path baseDestinationPath;
    if (xdgDataHome == nullptr)
    {
        auto home = getenv("HOME");
        baseDestinationPath = std::filesystem::path(home) / ".local" / "share";
    }
    else
    {
        baseDestinationPath = std::filesystem::path(xdgDataHome);
    }

    //
    // Copy icon file
    //
    bool foundIcon = true;
    std::filesystem::path iconDestinationPath;

    auto iconFileName = GetApplicationId() + ".png";
    auto iconFilePath = std::filesystem::path(mountPoint) / iconFileName;
    if (!std::filesystem::exists(iconFilePath))
    {
        iconFileName = GetApplicationId() + ".svg";
        iconFilePath = std::filesystem::path(mountPoint) / iconFileName;

        if (!std::filesystem::exists(iconFilePath))
        {
            iconFileName = GetApplicationId() + ".xpm";
            iconFilePath = std::filesystem::path(mountPoint) / iconFileName;

            if (!std::filesystem::exists(iconFilePath))
            {
                g_warning("Icon file not found.");
                foundIcon = false;
            }
        }
    }

    if (foundIcon)
    {
        iconFilePath = std::filesystem::canonical(iconFilePath);
        auto appDirUsrShare = std::filesystem::path(mountPoint) / "usr" / "share";

        auto iconFilePathIt = iconFilePath.begin();
        for (auto pathIt = appDirUsrShare.begin(); pathIt != appDirUsrShare.end(); ++pathIt)
        {
            if (iconFilePathIt == iconFilePath.end() || *pathIt != *iconFilePathIt)
            {
                break;
            }

            ++iconFilePathIt;
        }

        iconDestinationPath = baseDestinationPath;
        for (; iconFilePathIt != iconFilePath.end(); ++iconFilePathIt)
        {
            iconDestinationPath /= *iconFilePathIt;
        }
        std::filesystem::create_directories(iconDestinationPath.parent_path());
        std::filesystem::copy(iconFilePath, iconDestinationPath, std::filesystem::copy_options::overwrite_existing);
    }

    //
    // Modify and copy the desktop file
    //
    auto desktopFileName = GetApplicationId() + ".desktop";
    auto desktopFilePath = std::filesystem::path(mountPoint) / desktopFileName;
    auto desktopDestinationPath = baseDestinationPath / "applications" / desktopFileName;

    g_autoptr(GError) error = nullptr;
    g_autoptr(GKeyFile) key_file = g_key_file_new();

    if (!g_key_file_load_from_file(
                key_file, desktopFilePath.c_str(),
                static_cast<GKeyFileFlags>(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS), &error))
    {
        if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
            g_warning("Error loading key file: %s", error->message);
        return;
    }


    g_key_file_set_string(key_file, "Desktop Entry", "StartupWMClass", wmClass);

    if (foundIcon)
    {
        g_key_file_set_string(key_file, "Desktop Entry", "Icon", iconDestinationPath.c_str());
    }

    if (auto appImagePath = getenv("APPIMAGE"); appImagePath != nullptr)
    {
        g_key_file_set_string(key_file, "Desktop Entry", "Exec", appImagePath);
    }

    if (!g_key_file_save_to_file(key_file, desktopDestinationPath.c_str(), &error))
    {
        g_warning("Error saving key file: %s", error->message);
        return;
    }
}
