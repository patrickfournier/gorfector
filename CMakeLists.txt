cmake_minimum_required(VERSION 3.26)
project(zooscan)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 23)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

find_package(TIFF)

set(RESOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/zooscan.xml)
set(RESOURCE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zooscan.gresource.c)

add_custom_command(
        OUTPUT ${RESOURCE_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GLIB_COMPILE_RESOURCES}
        ARGS --target=${RESOURCE_OUTPUT} ${RESOURCE_FILE} --generate-source
        DEPENDS ${RESOURCE_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/resources/zooscan.css
        COMMENT "Compiling resources"
        VERBATIM
)

add_custom_target(
        resources ALL
        DEPENDS ${RESOURCE_OUTPUT}
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

set(SOURCE_FILES
        main.cpp

        ZooLib/Application.cpp
        ZooLib/ErrorDialog.cpp

        App.cpp
        DeviceSelector.cpp
        DeviceOptionsPanel.cpp
        DeviceOptionsState.cpp

        SaneDevice.cpp
        DeviceSelectorState.cpp
        PreviewPanel.cpp
        Writers/TiffWriter.cpp
        Writers/FileWriter.cpp
)

add_executable(zooscan ${SOURCE_FILES} ${RESOURCE_OUTPUT})

target_include_directories(zooscan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(zooscan PRIVATE -fexperimental-library -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-result)

set_source_files_properties(${RESOURCE_OUTPUT} PROPERTIES GENERATED TRUE)
add_dependencies(zooscan resources)

target_link_libraries(zooscan PRIVATE PkgConfig::GTK4)
target_link_libraries(zooscan PRIVATE libsane.so)
target_link_libraries(zooscan PRIVATE TIFF::TIFF)
target_link_libraries(zooscan PRIVATE nlohmann_json::nlohmann_json)

add_subdirectory(Tests)
