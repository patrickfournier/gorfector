#pragma once

#include <gtk/gtk.h>

#include "App.hpp"
#include "ViewUpdateObserver.hpp"
#include "ZooLib/CommandDispatcher.hpp"
#include "ZooLib/View.hpp"

namespace Gorfector
{
    class DeviceOptionsState;
    class OutputOptionsState;
    class OptionRewriter;

    class ScanOptionsPanel : public ZooLib::View
    {
    public:
        enum class Page
        {
            e_ScannerBasic,
            e_ScannerAdvanced,
            e_FileOutput,
        };

    private:
        constexpr static const char *k_ScannerBasicPageName = "basic";
        constexpr static const char *k_ScannerAdvancedPageName = "advanced";
        constexpr static const char *k_FileOutputPageName = "output";

        App *m_App;

        const int m_SaneInitId;
        const std::string m_DeviceName;
        DeviceOptionsState *m_DeviceOptions;
        OutputOptionsState *m_OutputOptions;

        ZooLib::CommandDispatcher m_Dispatcher;

        GtkWidget *m_RootWidget{};
        GtkWidget *m_OptionParent;
        GtkWidget *m_PageOutput{};
        GtkWidget *m_PageBasic{};
        GtkWidget *m_PageAdvanced{};

        ViewUpdateObserver<ScanOptionsPanel, DeviceOptionsState, OutputOptionsState> *m_OptionUpdateObserver;

        std::unordered_map<uint64_t, GtkWidget *> m_Widgets;

        GtkWidget *m_DestinationCombo{};
        GtkWidget *m_LocationEntryRow{};
        GtkWidget *m_CreateDirSwitch{};
        GtkWidget *m_FileNameEntry{};
        GtkWidget *m_IfFileExistsCombo{};

        static std::string SaneIntOrFixedToString(int value, const DeviceOptionValueBase *option);
        static const char *SaneUnitToString(SANE_Unit unit);

        OptionRewriter *m_Rewriter{};

        void BuildUI();
        std::vector<uint32_t> AddCommonOptions();
        void AddOtherScannerOptions(const std::vector<uint32_t> &excludeIndices);
        void AddOutputOptions();
        void OnBrowseButtonClicked(GtkWidget *widget);
        std::tuple<GtkWidget *, GtkWidget *> AddScannerOptionRow(
                uint64_t optionIndex, GtkWidget *parent, GtkWidget *pendingGroup, bool skipBasicOptions,
                bool skipAdvancedOptions);

        void
        AddCheckButtonForScannerOption(GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);
        void AddVectorRowForScannerOption(
                GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex, uint32_t valueIndex,
                bool multiValue);
        void
        AddStringRowForScannerOption(GtkWidget *parent, const DeviceOptionValueBase *option, uint32_t settingIndex);

        void OnScannerOptionCheckBoxChanged(GtkWidget *widget);
        void OnCheckBoxChanged(GtkWidget *widget);
        void OnScannerOptionDropDownChanged(GtkWidget *widget);
        void OnDropDownChanged(GtkWidget *widget);
        void OnScannerOptionNumericTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnScannerOptionStringTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnStringTextFieldChanged(GtkEventControllerFocus *focusController);
        void OnScannerOptionSpinButtonChanged(GtkWidget *widget);

    public:
        ScanOptionsPanel(int saneInitId, std::string deviceName, ZooLib::CommandDispatcher *parentDispatcher, App *app);

        ~ScanOptionsPanel() override;

        [[nodiscard]] int GetSaneInitId() const
        {
            return m_SaneInitId;
        }

        [[nodiscard]] const std::string &GetDeviceName() const
        {
            return m_DeviceName;
        }

        [[nodiscard]] GtkWidget *GetRootWidget() const override
        {
            return m_RootWidget;
        }

        [[nodiscard]] DeviceOptionsState *GetDeviceOptionsState() const
        {
            return m_DeviceOptions;
        }

        [[nodiscard]] OutputOptionsState *GetOutputOptionsState() const
        {
            return m_OutputOptions;
        }

        void SelectPage(Page page);

        void OnDirectorySelected(GFile *file);
        void Update(const std::vector<uint64_t> &lastSeenVersions) override;
    };
}
