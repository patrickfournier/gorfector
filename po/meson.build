xgettext = find_program('xgettext')
msgmerge = find_program('msgmerge')
msginit = find_program('msginit')
msgfmt = find_program('msgfmt')

po_files = [
    'fr.po',
]

input_files = [
    '../data/metainfo/com.patrickfournier.gorfector.metainfo.xml.in.in',
    '../data/com.patrickfournier.gorfector.desktop.in.in',

    '../src/Commands/DevMode/SetDumpSaneOptions.hpp',

    '../src/Commands/ActivateNetworkLookUp.hpp',
    '../src/Commands/ApplyPresetCommand.hpp',
    '../src/Commands/ChangeOptionCommand.hpp',
    '../src/Commands/CreatePresetCommand.hpp',
    '../src/Commands/CreateScanListItemCommand.hpp',
    '../src/Commands/DeletePresetCommand.hpp',
    '../src/Commands/DeleteScanItemCommand.hpp',
    '../src/Commands/LoadScanItemCommand.hpp',
    '../src/Commands/RefreshDeviceList.hpp',
    '../src/Commands/RenamePresetCommand.hpp',
    '../src/Commands/SelectDeviceCommand.hpp',
    '../src/Commands/SetCreateMissingDirectoriesCommand.hpp',
    '../src/Commands/SetFileExistsActionCommand.hpp',
    '../src/Commands/SetJpegQuality.hpp',
    '../src/Commands/SetPresetExpanded.hpp',
    '../src/Commands/SetOutputDestinationCommand.hpp',
    '../src/Commands/SetOutputDirectoryCommand.hpp',
    '../src/Commands/SetOutputFileNameCommand.hpp',
    '../src/Commands/SetPanCommand.hpp',
    '../src/Commands/SetPngCompressionLevel.hpp',
    '../src/Commands/SetScanAreaCommand.hpp',
    '../src/Commands/SetTiffCompression.hpp',
    '../src/Commands/SetTiffDeflateLevel.hpp',
    '../src/Commands/SetTiffJpegQuality.hpp',
    '../src/Commands/SetZoomCommand.hpp',
    '../src/Commands/ToggleUseScanList.hpp',

    '../src/Writers/FileWriter.cpp',
    '../src/Writers/FileWriter.hpp',
    '../src/Writers/JpegWriter.cpp',
    '../src/Writers/JpegWriter.hpp',
    '../src/Writers/JpegWriterState.hpp',
    '../src/Writers/PngWriter.cpp',
    '../src/Writers/PngWriter.hpp',
    '../src/Writers/PngWriterState.hpp',
    '../src/Writers/TiffWriter.cpp',
    '../src/Writers/TiffWriter.hpp',
    '../src/Writers/TiffWriterState.hpp',

    '../src/ZooLib/Application.cpp',
    '../src/ZooLib/Application.hpp',
    '../src/ZooLib/AppMenuBarBuilder.hpp',
    '../src/ZooLib/ChangesetBase.hpp',
    '../src/ZooLib/ChangesetManager.hpp',
    '../src/ZooLib/Command.hpp',
    '../src/ZooLib/CommandDispatcher.hpp',
    '../src/ZooLib/ErrorDialog.cpp',
    '../src/ZooLib/ErrorDialog.hpp',
    '../src/ZooLib/Gettext.hpp',
    '../src/ZooLib/GtkUtils.hpp',
    '../src/ZooLib/Observer.hpp',
    '../src/ZooLib/ObserverManager.hpp',
    '../src/ZooLib/PathUtils.cpp',
    '../src/ZooLib/PathUtils.hpp',
    '../src/ZooLib/SignalSupport.hpp',
    '../src/ZooLib/State.cpp',
    '../src/ZooLib/State.hpp',
    '../src/ZooLib/StateComponent.hpp',
    '../src/ZooLib/StringUtils.hpp',
    '../src/ZooLib/View.hpp',

    '../src/main.cpp',

    '../src/App.cpp',
    '../src/App.hpp',
    '../src/AppState.hpp',
    '../src/CurrentDeviceObserver.hpp',
    '../src/DeviceOptionsObserver.hpp',
    '../src/DeviceOptionsState.cpp',
    '../src/DeviceOptionsState.hpp',
    '../src/DeviceOptionValueBase.hpp',
    '../src/DeviceOptionValue.hpp',
    '../src/DeviceSelector.cpp',
    '../src/DeviceSelector.hpp',
    '../src/DeviceSelectorObserver.hpp',
    '../src/DeviceSelectorState.cpp',
    '../src/DeviceSelectorState.hpp',
    '../src/MultiScanProcess.cpp',
    '../src/MultiScanProcess.hpp',
    '../src/OptionRewriter.cpp',
    '../src/OptionRewriter.hpp',
    '../src/OutputOptionsState.hpp',
    '../src/Point.hpp',
    '../src/PreferencesView.cpp',
    '../src/PreferencesView.hpp',
    '../src/PresetCreateDialog.cpp',
    '../src/PresetPanel.cpp',
    '../src/PresetPanel.hpp',
    '../src/PresetPanelDialogs.hpp',
    '../src/PresetPanelState.hpp',
    '../src/PresetRenameDialog.cpp',
    '../src/PresetViewDialog.cpp',
    '../src/PreviewPanel.cpp',
    '../src/PreviewPanel.hpp',
    '../src/PreviewScanProcess.cpp',
    '../src/PreviewScanProcess.hpp',
    '../src/PreviewState.hpp',
    '../src/Rect.hpp',
    '../src/SaneDevice.hpp',
    '../src/SaneException.hpp',
    '../src/ScanListPanel.cpp',
    '../src/ScanListPanel.hpp',
    '../src/ScanListState.hpp',
    '../src/ScanOptionsPanel.cpp',
    '../src/ScanOptionsPanel.hpp',
    '../src/ScanProcess.cpp',
    '../src/ScanProcess.hpp',
    '../src/SingleScanProcess.hpp',
    '../src/ViewUpdateObserver.hpp',

    scanner_strings,
]

languages = []
foreach po_file : po_files
    languages += [po_file.split('.')[0]]
endforeach

update_pot = custom_target(
    'update_pot',
    output: gettext_package + '.pot',
    input: input_files,
    command: [
        'meson_helpers/update_pot.sh',
        '@CURRENT_SOURCE_DIR@',
        '@OUTPUT@',
        '@INPUT@'
    ],
)

update_all_po = custom_target(
    'update_all_po',
    input: update_pot,
    output: ['LINGUAS', po_files],
    command: [
        'meson_helpers/update_all_po.sh',
        '@CURRENT_SOURCE_DIR@',
        '@OUTDIR@',
        '@INPUT@',
        languages,
    ],
)

compile_all_mo = custom_target(
    'compile_all_mo',
    input: [update_all_po],
    output: 'locale',
    command: [
        'meson_helpers/compile_all_mo.sh',
        '@OUTDIR@',
        gettext_package,
        '@INPUT@',
    ],
)

mo_dep = declare_dependency(
    sources: [compile_all_mo],
)

install_subdir(
    compile_all_mo[0].full_path(),
    install_dir: datadir,
)
