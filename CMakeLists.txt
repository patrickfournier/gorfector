cmake_minimum_required(VERSION 3.26)
project(zooscan)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 23)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.14.0)
pkg_check_modules(ADW REQUIRED IMPORTED_TARGET libadwaita-1>=1.5.0)

find_package(TIFF)
find_package(JPEG)
find_package(PNG)

find_package (Intl REQUIRED)
find_package(Gettext REQUIRED)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
set(GETTEXT_OUTPUT_NAME "messages")

find_program(JQ_EXECUTABLE jq)
find_program(CAT_EXECUTABLE cat)

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
add_subdirectory(ZooLib/json)

set(SOURCE_FILES
        main.cpp

        ZooLib/Application.cpp
        ZooLib/ErrorDialog.cpp
        ZooLib/State.cpp

        App.cpp
        DeviceSelector.cpp
        DeviceOptionsState.cpp
        DeviceSelectorState.cpp
        OptionRewriter.cpp
        PreviewPanel.cpp
        ScanOptionsPanel.cpp

        Writers/FileWriter.cpp
        Writers/JpegWriter.cpp
        Writers/PngWriter.cpp
        Writers/TiffWriter.cpp
)

set(GETTEXT_SOURCE_FILES
        ${SOURCE_FILES}

        Commands/ChangeOptionCommand.hpp
        Commands/DeviceSelectorCommands.hpp
        Commands/DevMode/SetDumpSaneOptions.hpp
        Commands/SelectDeviceCommand.hpp
        Commands/SetCreateMissingDirectoriesCommand.hpp
        Commands/SetFileExistsActionCommand.hpp
        Commands/SetJpegQuality.hpp
        Commands/SetOutputDestinationCommand.hpp
        Commands/SetOutputDirectoryCommand.hpp
        Commands/SetOutputFileNameCommand.hpp
        Commands/SetPanCommand.hpp
        Commands/SetPngCompressionLevel.hpp
        Commands/SetScanAreaCommand.hpp
        Commands/SetScanMode.hpp
        Commands/SetTiffCompression.hpp
        Commands/SetTiffDeflateLevel.hpp
        Commands/SetTiffJpegQuality.hpp
        Commands/SetZoomCommand.hpp

        Writers/FileWriter.hpp
        Writers/JpegWriter.hpp
        Writers/JpegWriterState.hpp
        Writers/PngWriter.hpp
        Writers/PngWriterState.hpp
        Writers/TiffWriter.hpp
        Writers/TiffWriterState.hpp

        ZooLib/Application.hpp
        ZooLib/AppMenuBarBuilder.hpp
        ZooLib/ChangesetBase.hpp
        ZooLib/ChangesetManager.hpp
        ZooLib/Command.hpp
        ZooLib/CommandDispatcher.hpp
        ZooLib/ErrorDialog.hpp
        ZooLib/Gettext.hpp
        ZooLib/Observer.hpp
        ZooLib/ObserverManager.hpp
        ZooLib/SignalSupport.hpp
        ZooLib/State.hpp
        ZooLib/StateComponent.hpp
        ZooLib/View.hpp

        App.hpp
        AppState.hpp
        DeviceOptionsObserver.hpp
        DeviceOptionValueBase.hpp
        DeviceOptionValue.hpp
        DeviceSelector.hpp
        DeviceSelectorObserver.hpp
        DeviceSelectorState.hpp
        OptionRewriter.hpp
        OutputOptionsState.hpp
        Point.hpp
        PreferencesView.hpp
        PreviewPanel.hpp
        PreviewState.hpp
        Rect.hpp
        SaneDevice.hpp
        SaneException.hpp
        ScanOptionsPanel.hpp
        ViewUpdateObserver.hpp
)

if (GETTEXT_XGETTEXT_EXECUTABLE)

    # Extract strings from JSON files: title, description and string_list. Make a list of them and remove empty strings.
    set(JQ_FILTER '[.options[].title,.options[].description,.options[].string_list[]] | map(select((. | length) > 0))')

    message(DEBUG " xgettext: ${GETTEXT_XGETTEXT_EXECUTABLE}")
    add_custom_target(
            pot-update
            COMMENT "pot-update: Done."
            DEPENDS ${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot
    )
    add_custom_command(
            TARGET pot-update
            PRE_BUILD
            COMMAND
            ${GETTEXT_XGETTEXT_EXECUTABLE}
            --add-comments
            --from-code=utf-8
            --c++
            --force-po
            --output=${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot
            --keyword=_
            --sort-output
            --package-name=${GETTEXT_OUTPUT_NAME}
            ${GETTEXT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "pot-update: Pot file generated: ${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot"
    )
    add_custom_command(
            TARGET pot-update
            PRE_BUILD
            COMMAND
            ${CAT_EXECUTABLE}
            scanners/*/*.json
            | ${JQ_EXECUTABLE}
            ${JQ_FILTER}
            | ${GETTEXT_XGETTEXT_EXECUTABLE}
            --from-code=utf-8
            --c++
            --join-existing
            --output=${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot
            --extract-all
            --sort-output
            --package-name=${GETTEXT_OUTPUT_NAME}
            -
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "pot-update: Pot file generated: ${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot"
    )

endif (GETTEXT_XGETTEXT_EXECUTABLE)

if (GETTEXT_MSGMERGE_EXECUTABLE)

    message(DEBUG " msgmerge: ${GETTEXT_MSGMERGE_EXECUTABLE}")

    add_custom_target(
            pot-merge
            COMMENT "pot-merge: Done."
            DEPENDS ${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot
    )

    file(GLOB PO_FILES ${CMAKE_SOURCE_DIR}/locale/*/${GETTEXT_OUTPUT_NAME}.po)
    message(TRACE " PO_FILES: ${PO_FILES}")

    foreach(PO_FILE IN ITEMS ${PO_FILES})
        message(STATUS " Adding msgmerge for: ${PO_FILE}")
        add_custom_command(
                TARGET pot-merge
                PRE_BUILD
                COMMAND
                ${GETTEXT_MSGMERGE_EXECUTABLE}
                --update
                ${PO_FILE}
                ${CMAKE_SOURCE_DIR}/locale/${GETTEXT_OUTPUT_NAME}.pot
                COMMENT "pot-merge: ${PO_FILE}"
        )
    endforeach()

endif (GETTEXT_MSGMERGE_EXECUTABLE)

if (GETTEXT_MSGFMT_EXECUTABLE)

    message(DEBUG " msgmerge: ${GETTEXT_MSGFMT_EXECUTABLE}")
    file(GLOB PO_LANGS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/locale/*)
    message(TRACE " PO_LANGS: ${PO_LANGS}")

    add_custom_target(
            po-compile
            COMMENT "po-compile: Done."
    )

    foreach(PO_LANG IN ITEMS ${PO_LANGS})
        if(IS_DIRECTORY ${PO_LANG})
            message(STATUS " Adding msgfmt for: ${PO_LANG}")
            add_custom_command(
                    TARGET po-compile
                    PRE_BUILD
                    COMMAND
                    ${GETTEXT_MSGFMT_EXECUTABLE}
                    --output-file=LC_MESSAGES/${GETTEXT_OUTPUT_NAME}.mo
                    ${GETTEXT_OUTPUT_NAME}.po
                    WORKING_DIRECTORY ${PO_LANG}
                    COMMENT "po-compile: ${PO_LANG}"
            )
        endif()
    endforeach()

endif (GETTEXT_MSGFMT_EXECUTABLE)

add_executable(zooscan ${SOURCE_FILES} ${RESOURCE_OUTPUT})

target_include_directories(zooscan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(zooscan PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-result)

set_source_files_properties(${RESOURCE_OUTPUT} PROPERTIES GENERATED TRUE)
add_dependencies(zooscan resources)

target_link_libraries(zooscan PRIVATE PkgConfig::GTK4)
target_link_libraries(zooscan PRIVATE PkgConfig::ADW)
target_link_libraries(zooscan PRIVATE libsane.so)
target_link_libraries(zooscan PRIVATE TIFF::TIFF)
target_link_libraries(zooscan PRIVATE JPEG::JPEG)
target_link_libraries(zooscan PRIVATE PNG::PNG)
target_link_libraries(zooscan PRIVATE nlohmann_json::nlohmann_json)

add_subdirectory(Tests)
