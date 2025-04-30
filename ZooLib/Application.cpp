#include <adwaita.h>

#include "AppMenuBarBuilder.hpp"
#include "Application.hpp"
#include "SignalSupport.hpp"

ZooLib::Application::Application()
    : m_GtkApp(nullptr)
{
}

void ZooLib::Application::Initialize()
{
    m_GtkApp = adw_application_new(GetApplicationId().c_str(), GetApplicationFlags());
    ConnectGtkSignal(this, &Application::OnActivate, m_GtkApp, "activate");
}

ZooLib::Application::~Application()
{
    g_object_unref(m_GtkApp);
    m_GtkApp = nullptr;
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
    gtk_widget_add_tick_callback(
            m_MainWindow,
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *localApp = static_cast<Application *>(data);
                localApp->m_ObserverManager.NotifyObservers();
                return G_SOURCE_CONTINUE;
            },
            this, nullptr);
}
