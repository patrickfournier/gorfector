project(StandAloneTests)
add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ..)

set(TEST_FILES
        ObserverManager_tests.cpp
        StateComponent_tests.cpp
)

add_executable(zooscan_standalonetests ${TEST_FILES})

target_link_libraries(zooscan_standalonetests gtest gtest_main)
