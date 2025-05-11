gorfector_sources = [
    'App.cpp',
    'DeviceOptionsState.cpp',
    'DeviceSelector.cpp',
    'DeviceSelectorState.cpp',
    'main.cpp',
    'MultiScanProcess.cpp',
    'OptionRewriter.cpp',
    'PathUtils.cpp',
    'PreferencesView.cpp',
    'PresetCreateDialog.cpp',
    'PresetPanel.cpp',
    'PresetRenameDialog.cpp',
    'PresetViewDialog.cpp',
    'PreviewPanel.cpp',
    'PreviewScanProcess.cpp',
    'ScanListPanel.cpp',
    'ScanOptionsPanel.cpp',
    'ScanProcess.cpp',

    'Writers/FileWriter.cpp',
    'Writers/JpegWriter.cpp',
    'Writers/PngWriter.cpp',
    'Writers/TiffWriter.cpp',

    'ZooLib/Application.cpp',
    'ZooLib/ErrorDialog.cpp',
    'ZooLib/State.cpp',
]

executable(
    'gorfector',
    gorfector_sources,
    gorfector_resources,
    dependencies : [
        gtk4_dep,
        adw_dep,
        libtiff_dep,
        libjpeg_dep,
        libpng_dep,
        nlohmann_json_dep,
        libsane_dep,
    ],
    include_directories : [
        rootInclude,
        'Commands',
        'Writers',
        'ZooLib',
    ],
    cpp_args : cxxflags,
    install : true
)

subdir('Tests')
