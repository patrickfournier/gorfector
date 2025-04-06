#include "Application.hpp"
#include "SignalSupport.hpp"

ZooLib::Application::Application()
    : m_GtkApp(nullptr)
{
}

void ZooLib::Application::Initialize()
{
    m_GtkApp = gtk_application_new(GetApplicationId().c_str(), GetApplicationFlags());
    ConnectGtkSignal(this, &Application::OnActivate, m_GtkApp, "activate");
}

ZooLib::Application::~Application()
{
    g_object_unref(m_GtkApp);
    m_GtkApp = nullptr;
}

int ZooLib::Application::Run(int argc, char **argv) const
{
    return g_application_run(G_APPLICATION(m_GtkApp), argc, argv);
}

void ZooLib::Application::OnActivate(GtkApplication *app)
{
    m_MainWindow = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(m_MainWindow, GetMainWindowTitle().c_str());
    auto size = GetMainWindowSize();
    gtk_window_set_default_size(m_MainWindow, std::get<0>(size), std::get<1>(size));

    PopulateMainWindow();

    gtk_window_present(m_MainWindow);
    gtk_widget_add_tick_callback(
            GTK_WIDGET(m_MainWindow),
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *localApp = static_cast<Application *>(data);
                localApp->m_ObserverManager.NotifyObservers();
                return G_SOURCE_CONTINUE;
            },
            this, nullptr);
}
