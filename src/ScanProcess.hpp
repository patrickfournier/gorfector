#pragma once

#include <gtk/gtk.h>
#include <thread>

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
        class ScanThread
        {
            const SaneDevice *m_Device;
            SANE_Byte *m_Buffer;
            size_t m_BufferSize;
            size_t m_WrittenLength{};
            size_t m_ReadLength{};

            bool m_Abort{};
            bool m_Finished{};

        public:
            ScanThread(const SaneDevice *device, size_t bufferSize)
                : m_Device(device)
                , m_Buffer(new SANE_Byte[bufferSize])
                , m_BufferSize(bufferSize)
            {
            }

            ~ScanThread()
            {
                delete[] m_Buffer;
            }

            bool Copy(SANE_Byte *buffer, size_t maxLength, size_t &readLength)
            {
                if (buffer != nullptr && maxLength > 0)
                {
                    readLength = std::max(0uz, std::min(maxLength, m_WrittenLength - m_ReadLength));
                    if (readLength > 0)
                    {
                        std::memcpy(buffer, m_Buffer + m_ReadLength, readLength);
                        m_ReadLength += readLength;
                    }
                }

                return !m_Finished || m_ReadLength < m_WrittenLength;
            }

            [[nodiscard]] bool Finished() const
            {
                return m_Finished;
            }

            void RequestAbort()
            {
                m_Abort = true;
            }

            void operator()()
            {
                if (m_Buffer == nullptr || m_BufferSize <= 0)
                {
                    return;
                }

                auto done = false;
                while (!done && !m_Abort)
                {
                    SANE_Int readLength = 0;
                    SANE_Int maxLength = static_cast<SANE_Int>(std::min(
                            m_BufferSize - m_WrittenLength, static_cast<size_t>(std::numeric_limits<SANE_Int>::max())));
                    done = !m_Device->Read(m_Buffer + m_WrittenLength, maxLength, &readLength);
                    m_WrittenLength += readLength;
                }

                m_Finished = true;
            }
        };

        SaneDevice *m_Device;
        AppState *m_AppState;
        PreviewState *m_PreviewState;
        DeviceOptionsState *m_ScanOptions;
        OutputOptionsState *m_OutputOptions;
        GtkWidget *m_MainWindow;
        const std::function<void()> *m_FinishCallback;
        guint m_ScanCallbackId{};
        SANE_Parameters m_ScanParameters{};
        ScanThread *m_ScanThread{};

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
            size_t readLength = 0;
            SANE_Byte *readBuffer = nullptr;
            size_t maxReadLength = 0;
            GetBuffer(readBuffer, maxReadLength);

            if (readBuffer == nullptr || maxReadLength <= 0)
            {
                return false;
            }

            if (m_ScanThread == nullptr)
            {
                return false;
            }

            bool continueScan = m_ScanThread->Copy(readBuffer, maxReadLength, readLength);
            CommitBuffer(readLength);
            return continueScan;
        }

        virtual void GetBuffer(SANE_Byte *&outBuffer, size_t &outMaxReadLength)
        {
            outBuffer = nullptr;
            outMaxReadLength = 0;
        }

        virtual void CommitBuffer(size_t readLength)
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

            if (m_ScanThread != nullptr)
            {
                m_ScanThread->RequestAbort();
                while (!m_ScanThread->Finished())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                delete m_ScanThread;
                m_ScanThread = nullptr;
            }

            if (canceled && m_Device != nullptr)
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

            auto bufferSize = static_cast<size_t>(m_ScanParameters.bytes_per_line) * m_ScanParameters.lines;
            m_ScanThread = new ScanThread(m_Device, bufferSize);
            auto t(std::thread(std::ref(*m_ScanThread)));
            t.detach();

            InstallGtkCallback();

            if (m_PreviewState != nullptr)
            {
                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.InitProgress(GetProgressString(), 0UL, bufferSize);
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
