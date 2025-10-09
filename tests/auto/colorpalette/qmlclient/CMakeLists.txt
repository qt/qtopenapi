# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(qml-qt-colorpalette LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(qml-qt-colorpalette
    QMLTEST
    SOURCES
        colorpalettetest/main.cpp
    LIBRARIES
        Qt::Qml
        Qt::QmlNetwork
        Qt::QuickTest
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_autogen_tools_initial_setup(ColorpaletteClientQml)

qt_add_qml_module(qml-qt-colorpalette
    URI main.uri
    VERSION 1.0
    QML_FILES
        colorpalettetest/tst_colorpalette.qml
)

target_include_directories(qml-qt-colorpalette
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/client>
)

target_link_libraries(qml-qt-colorpalette PRIVATE ColorpaletteClientQml)
