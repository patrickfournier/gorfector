#pragma once

#include <gtk/gtk.h>

#include "AppState.hpp"
#include "DeviceOptionsState.hpp"
#include "OutputOptionsState.hpp"
#include "PreviewState.hpp"
#include "ZooLib/ErrorDialog.hpp"

namespace Gorfector
{
    class ScanProcess
    {
    protected:
        SaneDevice *m_Device;
        AppState *m_AppState;
        PreviewState *m_PreviewState;
        DeviceOptionsState *m_ScanOptions;
        OutputOptionsState *m_OutputOptions;
        GtkWidget *m_MainWindow;
        const std::function<void()> *m_FinishCallback;
        guint m_ScanCallbackId{};
        SANE_Parameters m_ScanParameters{};

        virtual std::string GetProgressString()
        {
            return "";
        }

        virtual void InstallGtkCallback();

        virtual bool AfterStartScanChecks()
        {
            return true;
        }

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

        virtual void GetBuffer(SANE_Byte *&outBuffer, int &outMaxReadLength)
        {
            outBuffer = nullptr;
            outMaxReadLength = 0;
        }

        virtual void CommitBuffer(int32_t readLength)
        {
            if (m_PreviewState != nullptr)
            {
                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.IncreaseProgress(readLength);
            }
        }

        virtual void Stop(bool canceled)
        {
            if (m_PreviewState != nullptr)
            {
                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.SetProgressCompleted();
            }

            if (m_Device != nullptr)
            {
                m_Device->CancelScan();
            }
        }

    public:
        ScanProcess(
                SaneDevice *device, PreviewState *previewState, AppState *appState, DeviceOptionsState *scanOptions,
                OutputOptionsState *outputOptions, GtkWidget *mainWindow, const std::function<void()> *finishCallback)
            : m_Device(device)
            , m_AppState(appState)
            , m_PreviewState(previewState)
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
            if (!m_Device->StartScan())
            {
                Stop(true);
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Failed to start scan"));
                return false;
            }

            if (!m_Device->GetParameters(&m_ScanParameters))
            {
                Stop(true);
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Failed to start scan"));
                return false;
            }

            if (!AfterStartScanChecks())
            {
                Stop(true);
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Failed to start scan"));
                return false;
            }

            InstallGtkCallback();

            if (m_PreviewState != nullptr)
            {
                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.InitProgress(
                        GetProgressString(), 0, m_ScanParameters.bytes_per_line * m_ScanParameters.lines);
            }

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
