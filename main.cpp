#include "App.hpp"

#include <csignal>

#include "ZooLib/Gettext.hpp"

static Gorfector::App *app;

void SignalHandler(int signal)
{
    delete app;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SignalHandler);
    signal(SIGILL, SignalHandler);
    signal(SIGABRT, SignalHandler);
    signal(SIGSEGV, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGKILL, SignalHandler);

    setlocale(LC_ALL, "");
    bind_textdomain_codeset("messages", "UTF-8");
    bindtextdomain("messages", "../locale/");
    textdomain("messages");

    app = Gorfector::App::Create(argc, argv);
    auto retVal = app->Run();
    delete app;
    return retVal;
}
