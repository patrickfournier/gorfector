project(StandAloneTests)
add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ..)

set(TEST_FILES
        ObserverManager_tests.cpp
        StateComponent_tests.cpp

        ../ZooLib/State.cpp
)

add_executable(zooscan_standalonetests ${TEST_FILES})

target_link_libraries(zooscan_standalonetests gtest gtest_main)

enable_testing()
add_test(NAME zooscan_test
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/zooscan_standalonetests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
