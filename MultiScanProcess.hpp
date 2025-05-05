#pragma once

#include "PathUtils.hpp"
#include "ScanProcess.hpp"
#include "SingleScanProcess.hpp"
#include "Writers/FileWriter.hpp"

namespace Gorfector
{
    class MultiScanProcess : public SingleScanProcess
    {
        ScanListState *m_ScanListState;
        size_t m_CurrentScanIndex{};
        bool m_IsFinished{};

        std::string GetProgressString() override
        {
            return std::to_string(m_CurrentScanIndex + 1) + "/" + std::to_string(m_ScanListState->GetScanListSize()) +
                   ": " + SingleScanProcess::GetProgressString();
        }

        bool ComputeFileName()
        {
            const auto &dirPath = m_OutputOptions->GetOutputDirectory();
            const auto &fileName = m_OutputOptions->GetOutputFileName();

            if (!std::filesystem::exists(dirPath) && m_OutputOptions->GetCreateMissingDirectories())
            {
                std::filesystem::create_directory(dirPath);
            }

            if (!std::filesystem::exists(dirPath))
            {
                return false;
            }

            m_ImageFilePath = dirPath / fileName;
            if (std::filesystem::exists(m_ImageFilePath))
            {
                switch (m_OutputOptions->GetFileExistsAction())
                {
                    case OutputOptionsState::FileExistsAction::e_Cancel:
                    {
                        return false;
                    }
                    case OutputOptionsState::FileExistsAction::e_IncrementCounter:
                    {
                        IncrementPath(m_ImageFilePath);
                        if (std::filesystem::exists(m_ImageFilePath))
                        {
                            return false;
                        }
                        break;
                    }
                    case OutputOptionsState::FileExistsAction::e_Overwrite:
                    {
                        break;
                    }
                }
            }

            return true;
        }

        bool LoadSettings() override
        {
            if (m_CurrentScanIndex >= m_ScanListState->GetScanListSize())
            {
                return false;
            }

            auto scannerSettings = m_ScanListState->GetScannerSettings(m_CurrentScanIndex);
            auto outputSettings = m_ScanListState->GetOutputSettings(m_CurrentScanIndex);

            if (scannerSettings == nullptr || outputSettings == nullptr || scannerSettings->empty() ||
                outputSettings->empty())
            {
                return false;
            }

            auto deviceOptionsUpdater = DeviceOptionsState::Updater(m_ScanOptions);
            deviceOptionsUpdater.ApplySettings(*scannerSettings);

            auto outputOptionsUpdater = OutputOptionsState::Updater(m_OutputOptions);
            outputOptionsUpdater.ApplySettings(*outputSettings);

            if (ComputeFileName())
            {
                m_FileWriter = FileWriter::GetFileWriterForPath(m_ImageFilePath);
                return m_FileWriter != nullptr;
            }

            return false;
        }

    protected:
        void InstallGtkCallback() override;

        void Stop(bool canceled) override
        {
            SingleScanProcess::Stop(canceled);

            ++m_CurrentScanIndex;
            if (canceled || m_CurrentScanIndex >= m_ScanListState->GetScanListSize())
            {
                m_IsFinished = true;
            }
            else
            {
                Start();
            }
        }

    public:
        MultiScanProcess(
                SaneDevice *device, ScanListState *scanListState, PreviewState *previewState, AppState *appState,
                DeviceOptionsState *scanOptions, OutputOptionsState *outputOptions, GtkWidget *mainWindow,
                const std::function<void()> *finishCallback)
            : SingleScanProcess(
                      device, previewState, appState, scanOptions, outputOptions, mainWindow, nullptr, "",
                      finishCallback)
            , m_ScanListState(scanListState)
        {
        }

        bool IsFinished() const
        {
            return m_IsFinished;
        }
    };
}
