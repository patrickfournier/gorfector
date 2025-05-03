#pragma once

#include "ScanProcess.hpp"

namespace Gorfector
{

    class PreviewScanProcess final : public ScanProcess
    {
        PreviewState *m_PreviewState;

        void SetPreviewOptions() const;
        void RestoreOptionsAfterPreview() const;

    protected:
        bool Update() override
        {
            if (m_PreviewState == nullptr)
            {
                return false;
            }

            return ScanProcess::Update();
        }

        void Stop(bool canceled) override
        {
            if (m_PreviewState)
            {
                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.SetProgressCompleted();
            }

            ScanProcess::Stop(canceled);

            RestoreOptionsAfterPreview();

            auto updater = AppState::Updater(m_AppState);
            updater.SetIsPreviewing(false);
        }

        bool AfterStartScanChecks() override
        {
            SANE_Parameters scanParameters{};
            m_Device->GetParameters(&scanParameters);
            if (scanParameters.format != SANE_FRAME_GRAY && scanParameters.format != SANE_FRAME_RGB)
            {
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
                return false;
            }

            return true;
        }

        void GetBuffer(SANE_Byte *&outBuffer, int &outMaxReadLength) override
        {
            auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
            previewPanelUpdater.GetReadBuffer(outBuffer, outMaxReadLength);
        }

        void CommitBuffer(int32_t readLength) override
        {
            auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
            previewPanelUpdater.CommitReadBuffer(readLength);
            previewPanelUpdater.IncreaseProgress(readLength);
        }

    public:
        PreviewScanProcess(
                SaneDevice *device, PreviewState *previewState, AppState *appState, DeviceOptionsState *scanOptions,
                OutputOptionsState *outputOptions, GtkWidget *mainWindow, const std::function<void()> *finishCallback)
            : ScanProcess(device, appState, scanOptions, outputOptions, mainWindow, finishCallback)
            , m_PreviewState(previewState)
        {
        }

        bool Start() override
        {
            auto updater = AppState::Updater(m_AppState);
            updater.SetIsPreviewing(true);

            SetPreviewOptions();

            if (!ScanProcess::Start())
            {
                // Stop() has already been called
                return false;
            }

            if (m_PreviewState != nullptr)
            {
                SANE_Parameters scanParameters{};
                m_Device->GetParameters(&scanParameters);

                auto previewPanelUpdater = PreviewState::Updater(m_PreviewState);
                previewPanelUpdater.PrepareForScan(
                        scanParameters.pixels_per_line, scanParameters.bytes_per_line, scanParameters.lines,
                        scanParameters.depth, scanParameters.format);
                previewPanelUpdater.InitProgress(
                        std::string(), 0, scanParameters.bytes_per_line * scanParameters.lines);
            }

            return true;
        }
    };

}
