#include "MultiScanProcess.hpp"

static void DestroyProcess(gpointer data)
{
    auto *scanProcess = static_cast<Gorfector::MultiScanProcess *>(data);

    if (scanProcess->IsFinished())
    {
        scanProcess->NotifyFinish();
        delete scanProcess;
    }
}

void Gorfector::MultiScanProcess::InstallGtkCallback()
{
    m_ScanCallbackId = gtk_widget_add_tick_callback(
            m_MainWindow,
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *scanProcess = static_cast<MultiScanProcess *>(data);
                if (scanProcess->Update())
                {
                    return G_SOURCE_CONTINUE;
                }

                scanProcess->Stop(false);
                return G_SOURCE_REMOVE;
            },
            this, DestroyProcess);
}
