#include "App.hpp"

#include <csignal>

static ZooScan::App *app;

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

    app = new ZooScan::App(argc, argv);
    app->Initialize();
    auto retVal = app->Run();
    delete app;
    app = nullptr;
    return retVal;
}
