
#include <csignal>

#include "App.hpp"
#include "ZooLib/Gettext.hpp"
#include "ZooLib/PathUtils.hpp"
#include "config.h"

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
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    bindtextdomain(GETTEXT_PACKAGE, ZooLib::RelocatePath(GNOMELOCALEDIR).c_str());
    textdomain(GETTEXT_PACKAGE);

    app = Gorfector::App::Create(argc, argv);
    auto retVal = app->Run();
    delete app;
    return retVal;
}
