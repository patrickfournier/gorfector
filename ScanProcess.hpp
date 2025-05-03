#pragma once

#include <gtk/gtk.h>

#include "AppState.hpp"
#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"
#include "PreviewState.hpp"
#include "ZooLib/ErrorDialog.hpp"

namespace Gorfector
{
    void DestroyProcess(gpointer data);

    class ScanProcess
    {
    protected:
        SaneDevice *m_Device;
        AppState *m_AppState;
        DeviceOptionsState *m_ScanOptions;
        OutputOptionsState *m_OutputOptions;
        GtkWidget *m_MainWindow;
        const std::function<void()> *m_FinishCallback;
        guint m_ScanCallbackId{};

        virtual bool Update()
        {
            SANE_Int readLength = 0;
            SANE_Byte *readBuffer = nullptr;
            int maxReadLength = 0;
            GetBuffer(readBuffer, maxReadLength);

            if (readBuffer == nullptr || maxReadLength <= 0)
            {
                return false;
            }

            bool continueScan = m_Device->Read(readBuffer, maxReadLength, &readLength);
            CommitBuffer(readLength);
            return continueScan;
        }

        virtual void Stop(bool canceled)
        {
            if (m_Device != nullptr)
            {
                m_Device->CancelScan();
            }
        }

        virtual bool AfterStartScanChecks()
        {
            // This function can be overridden to perform additional actions after starting the scan.
            return true;
        }

        virtual void GetBuffer(SANE_Byte *&outBuffer, int &outMaxReadLength)
        {
            outBuffer = nullptr;
            outMaxReadLength = 0;
        }

        virtual void CommitBuffer(int32_t readLength)
        {
            // This function can be overridden to commit the read buffer.
        }

    public:
        ScanProcess(
                SaneDevice *device, AppState *appState, DeviceOptionsState *scanOptions,
                OutputOptionsState *outputOptions, GtkWidget *mainWindow, const std::function<void()> *finishCallback)
            : m_Device(device)
            , m_AppState(appState)
            , m_ScanOptions(scanOptions)
            , m_OutputOptions(outputOptions)
            , m_MainWindow(mainWindow)
            , m_FinishCallback(finishCallback)
        {
        }

        virtual ~ScanProcess()
        {
            delete m_FinishCallback;
        }

        virtual bool Start()
        {
            try
            {
                m_Device->StartScan();
            }
            catch (const std::runtime_error &e)
            {
                Stop(true);
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), e.what());
                return false;
            }

            if (!AfterStartScanChecks())
            {
                Stop(true);
                return false;
            }

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

            return true;
        }

        void Cancel()
        {
            Stop(true);

            if (m_ScanCallbackId != 0)
            {
                auto scanCallbackId = m_ScanCallbackId;
                m_ScanCallbackId = 0;

                gtk_widget_remove_tick_callback(GTK_WIDGET(m_MainWindow), scanCallbackId);
                // `this` is now deleted (by DestroyProcess()).
            }
        }

        void NotifyFinish() const
        {
            (*m_FinishCallback)();
        }
    };
}
