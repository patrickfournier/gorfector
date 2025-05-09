project(StandAloneTests)
enable_testing()

find_library(XDO_LIB xdo REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.14.0)
pkg_check_modules(ADW REQUIRED IMPORTED_TARGET libadwaita-1>=1.5.0)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ..)

set(TEST_FILES
        ../ZooLib/Application.cpp
        ../ZooLib/State.cpp

        ZooLib/Application_tests.cpp
        ZooLib/AppMenuBarBuilder_tests.cpp
        ZooLib/ChangesetManager_tests.cpp
        ZooLib/CommandDispatcher_tests.cpp
        ZooLib/GtkUtils_tests.cpp
        ZooLib/ObserverManager_tests.cpp
        ZooLib/StateComponent_tests.cpp
        ZooLib/State_tests.cpp
        ZooLib/StringUtils_tests.cpp
        ZooLib/View_tests.cpp

        main.cpp
        TestsSupport/Applications.hpp
        TestsSupport/Commands.cpp
)

add_executable(gorfector_standalonetests ${TEST_FILES})

target_link_libraries(gorfector_standalonetests PRIVATE gtest)
target_link_libraries(gorfector_standalonetests PRIVATE PkgConfig::GTK4)
target_link_libraries(gorfector_standalonetests PRIVATE PkgConfig::ADW)
target_link_libraries(gorfector_standalonetests PRIVATE "${XDO_LIB}")

include(GoogleTest)
gtest_discover_tests(gorfector_standalonetests DISCOVERY_MODE PRE_TEST)
