#include "ScanProcess.hpp"

static void DestroyProcess(gpointer data)
{
    auto *scanProcess = static_cast<Gorfector::ScanProcess *>(data);
    scanProcess->NotifyFinish();
    delete scanProcess;
}

void Gorfector::ScanProcess::InstallGtkCallback()
{
    m_ScanCallbackId = gtk_widget_add_tick_callback(
            m_MainWindow,
            [](GtkWidget *widget, GdkFrameClock *frameClock, gpointer data) -> gboolean {
                auto *scanProcess = static_cast<ScanProcess *>(data);
                if (scanProcess->Update())
                {
                    return G_SOURCE_CONTINUE;
                }

                scanProcess->Stop(false);
                return G_SOURCE_REMOVE;
            },
            this, DestroyProcess);
}
