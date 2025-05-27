config_dep = declare_dependency(
    sources: [configure_file(output: 'config.h', configuration: conf)],
    include_directories: '.',
)

gorfector_sources = [
    'App.cpp',
    'DeviceOptionsState.cpp',
    'DeviceSelector.cpp',
    'DeviceSelectorState.cpp',
    'main.cpp',
    'MultiScanProcess.cpp',
    'OptionRewriter.cpp',
    'PreferencesView.cpp',
    'PresetCreateDialog.cpp',
    'PresetPanel.cpp',
    'PresetUpdateDialog.cpp',
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
    'ZooLib/PathUtils.cpp',
    'ZooLib/State.cpp',
]

root_include = include_directories('.')

generated_sources = [
    gorfector_resources,
]

executable(
    'gorfector',
    gorfector_sources,
    generated_sources,
    dependencies : [
        gtk4_dep,
        adw_dep,
        libtiff_dep,
        libjpeg_dep,
        libpng_dep,
        nlohmann_json_dep,
        libsane_dep,
        config_dep,
        mo_dep,
    ],
    include_directories : [
        root_include,
        'Commands',
        'Writers',
        'ZooLib',
    ],
    cpp_args : cxxflags,
    install : true
)

subdir('Tests')
