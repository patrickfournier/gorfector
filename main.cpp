#include "App.h"

int main()
{
    auto *app = new ZooScan::App();
    app->Initialize();
    return app->Run();
}
