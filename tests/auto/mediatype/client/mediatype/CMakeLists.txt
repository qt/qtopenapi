# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-mediatype LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET mediatype-server-go)
    message(WARNING "No mediatype-server-go server in the build.")
    return()
endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_internal_add_test(cpp-qt-mediatype
    SOURCES
        MediaType.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(MediaTypeClient)

# Resources:
set(cpp_qt_mediatype_resource_files
    "file-for-uploading.txt"
    "usericon.png"
)

qt_internal_add_resource(cpp-qt-mediatype "cpp-qt-mediatype"
    PREFIX
        "/"
    FILES
        ${cpp_qt_mediatype_resource_files}
)

target_compile_definitions(cpp-qt-mediatype
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:mediatype-server-go>"
)

target_include_directories(cpp-qt-mediatype
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_link_libraries(cpp-qt-mediatype PRIVATE MediaTypeClient CommonMediaTypeClient)
add_dependencies(cpp-qt-mediatype mediatype-server-go)
