#include "App.h"

int main()
{
    ZooScan::App *app;
    int status;

    try
    {
        app = new ZooScan::App();
        app->Initialize();
        status = app->Run();
    }
    catch (...)
    {
        delete app;
        throw;
    }
    return status;
}
