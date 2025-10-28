# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-colorpalette-app LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_internal_add_test(cpp-qt-colorpalette-app
    SOURCES
        ColorPaletteTests.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(ColorpaletteClient)

target_link_libraries(cpp-qt-colorpalette-app PRIVATE ColorpaletteClient CommonColorpaletteClient)

target_include_directories(cpp-qt-colorpalette-app
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

if(WIN32)
    add_custom_command(TARGET cpp-qt-colorpalette-app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CommonColorpaletteClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ColorpaletteClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()

