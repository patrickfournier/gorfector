gtest_dep = dependency('gtest', required: true)
xdo_dep = cx.find_library('xdo', required: true)

gorfector_tests_sources = [
    '../Writers/FileWriter.cpp',
    '../Writers/JpegWriter.cpp',
    '../Writers/PngWriter.cpp',
    '../Writers/TiffWriter.cpp',

    '../ZooLib/Application.cpp',
    '../ZooLib/State.cpp',

    '../DeviceOptionsState.cpp',

    'TestsSupport/Commands.cpp',
    'TestsSupport/CompareFiles.cpp',

    'ZooLib/Application_tests.cpp',
    'ZooLib/AppMenuBarBuilder_tests.cpp',
    'ZooLib/ChangesetManager_tests.cpp',
    'ZooLib/CommandDispatcher_tests.cpp',
    'ZooLib/GtkUtils_tests.cpp',
    'ZooLib/ObserverManager_tests.cpp',
    'ZooLib/State_tests.cpp',
    'ZooLib/StateComponent_tests.cpp',
    'ZooLib/StringUtils_tests.cpp',
    'ZooLib/View_tests.cpp',

    'JpegWriter_tests.cpp',
    'PngWriter_tests.cpp',
    'TiffWriter_tests.cpp',

    'main.cpp',
]

test_data = install_subdir(
    'Data',
    install_dir: join_paths(get_option('datadir'), app_id, 'Tests'),
)

test_exe = executable(
    'gorfector-tests',
    gorfector_tests_sources,
    dependencies : [
        gtk4_dep,
        adw_dep,
        libtiff_dep,
        libjpeg_dep,
        libpng_dep,
        nlohmann_json_dep,
        libsane_dep,
        gtest_dep,
        xdo_dep,
    ],
    include_directories : [
        root_include,
        'TestsSupport',
        'ZooLib',
        '..',
    ],
    cpp_args : cxxflags,
    install : true
)

test_data_dir = join_paths(meson.source_root(), 'src', 'Tests', 'Data')
test('gorfector tests', test_exe, args: ['--gtest_color=yes', '--data_dir=' + test_data_dir], verbose: true)
