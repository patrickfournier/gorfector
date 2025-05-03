#include "ScanProcess.hpp"

void Gorfector::DestroyProcess(gpointer data)
{
    auto *scanProcess = static_cast<ScanProcess *>(data);
    scanProcess->NotifyFinish();
    delete scanProcess;
}
