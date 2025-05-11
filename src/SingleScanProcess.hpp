#pragma once

#include "ScanProcess.hpp"
#include "Writers/FileWriter.hpp"

namespace Gorfector
{
    class SingleScanProcess : public ScanProcess
    {
    protected:
        FileWriter *m_FileWriter;
        std::filesystem::path m_ImageFilePath;

        SANE_Byte *m_Buffer{};
        int32_t m_BufferSize{};
        int32_t m_WriteOffset{};

        virtual bool LoadSettings()
        {
            return true;
        }

        std::string GetProgressString() override
        {
            return m_ImageFilePath.filename();
        }

        bool AfterStartScanChecks() override
        {
            if (m_ScanParameters.format != SANE_FRAME_GRAY && m_ScanParameters.format != SANE_FRAME_RGB)
            {
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported format"));
                return false;
            }

            if (m_ScanParameters.depth != 1 && m_ScanParameters.depth != 8 && m_ScanParameters.depth != 16)
            {
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported depth"));
                return false;
            }

            if (m_FileWriter == nullptr)
            {
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), _("Unsupported file format"));
                return false;
            }

            if (auto error = m_FileWriter->CreateFile(m_ImageFilePath, m_ScanOptions, m_ScanParameters);
                error != FileWriter::Error::None)
            {
                auto errorString = std::string(_("Failed to create file: ")) + m_FileWriter->GetError(error);
                ZooLib::ShowUserError(ADW_APPLICATION_WINDOW(m_MainWindow), errorString);
                return false;
            }

            return true;
        }

        void GetBuffer(SANE_Byte *&outBuffer, int &outMaxReadLength) override
        {
            outBuffer = m_Buffer != nullptr ? m_Buffer + m_WriteOffset : nullptr;
            outMaxReadLength = m_BufferSize - m_WriteOffset;
        }

        void CommitBuffer(int32_t readLength) override
        {
            ScanProcess::CommitBuffer(readLength);

            auto availableBytes = m_WriteOffset + readLength;
            auto availableLines = availableBytes / m_ScanParameters.bytes_per_line;
            auto savedBytes = m_FileWriter->AppendBytes(m_Buffer, availableLines, m_ScanParameters);
            if (static_cast<int32_t>(savedBytes) < availableBytes)
            {
                memmove(m_Buffer, m_Buffer + savedBytes, availableBytes - savedBytes);
                m_WriteOffset = availableBytes - static_cast<int32_t>(savedBytes);
            }
            else
            {
                m_WriteOffset = 0;
            }
        }

        void Stop(bool canceled) override
        {
            ScanProcess::Stop(canceled);

            SendImageToDestination(canceled);

            auto updater = AppState::Updater(m_AppState);
            updater.SetIsScanning(false);

            free(m_Buffer);
            m_Buffer = nullptr;
            m_BufferSize = 0;
            m_WriteOffset = 0;
        }

        void SendImageToDestination(bool canceled)
        {
            if (m_FileWriter != nullptr)
            {
                if (canceled)
                {
                    m_FileWriter->CancelFile();
                }
                else
                {
                    m_FileWriter->CloseFile();
                }
                m_FileWriter = nullptr;
            }

            if (!canceled && std::filesystem::exists(m_ImageFilePath))
            {
                auto destination = m_OutputOptions->GetOutputDestination();
                if (destination == OutputOptionsState::OutputDestination::e_Email)
                {
                    auto command = "xdg-email --attach " + m_ImageFilePath.string();
                    std::system(command.c_str());
                }
                else if (destination == OutputOptionsState::OutputDestination::e_Printer)
                {
                    auto printDialog = gtk_print_dialog_new();
                    auto imageFile = g_file_new_for_path(m_ImageFilePath.c_str());
                    gtk_print_dialog_print_file(
                            printDialog, GTK_WINDOW(m_MainWindow), nullptr, imageFile, nullptr, nullptr, nullptr);
                }
            }
        }

    public:
        SingleScanProcess(
                SaneDevice *device, PreviewState *previewState, AppState *appState, DeviceOptionsState *scanOptions,
                OutputOptionsState *outputOptions, GtkWidget *mainWindow, FileWriter *fileWriter,
                std::filesystem::path imageFilePath, const std::function<void()> *finishCallback)
            : ScanProcess(device, previewState, appState, scanOptions, outputOptions, mainWindow, finishCallback)
            , m_FileWriter(fileWriter)
            , m_ImageFilePath(std::move(imageFilePath))
        {
        }

        ~SingleScanProcess() override
        {
            if (m_Buffer != nullptr)
            {
                free(m_Buffer);
            }
        }

        bool Start() override
        {
            auto updater = AppState::Updater(m_AppState);
            updater.SetIsScanning(true);

            if (!LoadSettings())
            {
                updater.SetIsScanning(false);
                return false;
            }

            if (!ScanProcess::Start())
            {
                // Stop() has already been called
                updater.SetIsScanning(false);
                return false;
            }

            auto linesIn1MB = 1024 * 1024 / m_ScanParameters.bytes_per_line;
            m_BufferSize = m_ScanParameters.bytes_per_line * linesIn1MB;
            m_Buffer = static_cast<SANE_Byte *>(calloc(m_BufferSize, sizeof(SANE_Byte)));
            m_WriteOffset = 0;

            return true;
        }
    };
}
