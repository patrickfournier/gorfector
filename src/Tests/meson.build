gtest_dep = dependency('gtest', required: true)
xdo_dep = cx.find_library('xdo', required: true)

gorfector_tests_sources = [
    '../ZooLib/Application.cpp',
    '../ZooLib/State.cpp',

    'TestsSupport/Commands.cpp',

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

    'main.cpp',
]

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
        rootInclude,
        'TestsSupport',
        'ZooLib',
        '..',
    ],
    cpp_args : cxxflags,
    install : true
)

test('gorfector tests', test_exe, args: ['--gtest_color=yes'], verbose: true)
