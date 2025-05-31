#pragma once

#include <gtk/gtk.h>
#include <list>
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
            size_t m_ImageSize;
            const size_t m_ChunkSize{};

            std::list<SANE_Byte *> m_FreeChunks{};
            std::list<SANE_Byte *> m_UsedChunks{};

            std::list<SANE_Byte *>::iterator m_CurrentWriteChunk{};
            size_t m_WritePos{}; // 0 - m_ChunkSize

            std::list<SANE_Byte *>::iterator m_CurrentReadChunk{};
            size_t m_ReadPos{}; // 0 - m_ChunkSize

            size_t m_TotalBytesWritten{}; // 0 - m_ImageSize
            size_t m_TotalBytesRead{}; // 0 - m_ImageSize

            bool m_AbortRequested{};
            bool m_Finished{};

            std::mutex m_Mutex{};

        public:
            ScanThread(const SaneDevice *device, size_t imageSize)
                : m_Device(device)
                , m_ImageSize(imageSize)
                , m_ChunkSize(std::min(imageSize, static_cast<size_t>(2 * 1024 * 1024)))
            {
                auto chunk = new SANE_Byte[m_ChunkSize];
                m_UsedChunks.push_back(chunk);

                m_CurrentWriteChunk = m_UsedChunks.begin();
                m_WritePos = 0;

                m_CurrentReadChunk = m_UsedChunks.begin();
                m_ReadPos = 0;
            }

            ~ScanThread()
            {
                for (auto buffer: m_FreeChunks)
                {
                    delete[] buffer;
                }
                m_FreeChunks.clear();

                for (auto buffer: m_UsedChunks)
                {
                    delete[] buffer;
                }
                m_UsedChunks.clear();
            }

            /// \returns true if there is still data to read
            bool Copy(SANE_Byte *destBuffer, size_t maxLength, size_t &readLength)
            {
                if (destBuffer == nullptr || maxLength <= 0)
                {
                    readLength = 0;
                    return !m_Finished || m_TotalBytesRead < m_TotalBytesWritten;
                }

                std::lock_guard lock(m_Mutex);

                if (m_CurrentReadChunk == m_UsedChunks.end())
                {
                    m_CurrentReadChunk = m_UsedChunks.begin(); // if empty, == end()
                    m_ReadPos = 0;
                }

                if (m_CurrentReadChunk == m_UsedChunks.end())
                {
                    // No data to read for now
                    readLength = 0;
                    return !m_Finished || m_TotalBytesRead < m_TotalBytesWritten;
                }

                auto dataAvailable = 0uz;
                if (m_CurrentWriteChunk == m_CurrentReadChunk)
                {
                    dataAvailable = m_WritePos - m_ReadPos;
                }
                else
                {
                    dataAvailable = m_ChunkSize - m_ReadPos;
                }

                readLength = std::min(maxLength, dataAvailable);
                if (readLength > 0)
                {
                    std::memcpy(destBuffer, *m_CurrentReadChunk + m_ReadPos, readLength);
                    m_ReadPos += readLength;
                    m_TotalBytesRead += readLength;
                }

                if (m_ReadPos == m_ChunkSize)
                {
                    m_FreeChunks.push_back(*m_CurrentReadChunk);
                    m_CurrentReadChunk = m_UsedChunks.erase(m_CurrentReadChunk);
                    m_ReadPos = 0;
                }

                return !m_Finished || m_TotalBytesRead < m_TotalBytesWritten;
            }

            [[nodiscard]] bool Finished() const
            {
                return m_Finished;
            }

            void RequestAbort()
            {
                m_AbortRequested = true;
            }

            void operator()()
            {
                if (m_CurrentWriteChunk == m_UsedChunks.end() || m_ImageSize <= 0)
                {
                    return;
                }

                auto done = false;
                while (!done && !m_AbortRequested)
                {
                    SANE_Int readLength = 0;
                    SANE_Int maxLength = static_cast<SANE_Int>(std::min(
                            m_ChunkSize - m_WritePos, static_cast<size_t>(std::numeric_limits<SANE_Int>::max())));
                    done = !m_Device->Read(*m_CurrentWriteChunk + m_WritePos, maxLength, &readLength);

                    {
                        std::lock_guard lock(m_Mutex);

                        m_WritePos += readLength;
                        m_TotalBytesWritten += readLength;

                        if (m_WritePos == m_ChunkSize)
                        {
                            m_WritePos = 0;
                            m_CurrentWriteChunk = std::next(m_CurrentWriteChunk);

                            if (m_CurrentWriteChunk == m_UsedChunks.end())
                            {
                                if (m_FreeChunks.empty())
                                {
                                    auto newChunk = new SANE_Byte[m_ChunkSize];
                                    m_UsedChunks.push_back(newChunk);
                                    m_CurrentWriteChunk = std::prev(m_UsedChunks.end());
                                }
                                else
                                {
                                    m_CurrentWriteChunk = m_FreeChunks.begin();
                                    m_UsedChunks.push_back(*m_CurrentWriteChunk);
                                    m_FreeChunks.erase(m_CurrentWriteChunk);
                                    m_CurrentWriteChunk = std::prev(m_UsedChunks.end());
                                }
                            }
                        }
                    }
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
            if (m_ScanThread == nullptr)
            {
                auto bufferSize = static_cast<size_t>(m_ScanParameters.bytes_per_line) * m_ScanParameters.lines;
                m_ScanThread = new ScanThread(m_Device, bufferSize);
                auto t(std::thread(std::ref(*m_ScanThread)));
                t.detach();

                return true;
            }

            size_t readLength = 0;
            SANE_Byte *readBuffer = nullptr;
            size_t maxReadLength = 0;
            GetBuffer(readBuffer, maxReadLength);

            if (readBuffer == nullptr || maxReadLength <= 0)
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
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Failed to start scan."));
                return false;
            }

            if (!m_Device->GetParameters(&m_ScanParameters))
            {
                Stop(true);
                ZooLib::ShowUserError(
                        ADW_APPLICATION_WINDOW(m_MainWindow), _("Failed to start scan: cannot get parameters."));
                return false;
            }

            if (!AfterStartScanChecks())
            {
                Stop(true);
                // `AfterStartScanChecks()` already shows an error dialog
                return false;
            }

            auto bufferSize = static_cast<size_t>(m_ScanParameters.bytes_per_line) * m_ScanParameters.lines;
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
