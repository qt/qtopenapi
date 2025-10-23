# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-operation-parameters LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET op-server-go)
    message(WARNING "No op-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-operation-parameters
    SOURCES
        OperationParameters.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(OperationParametersClient)

target_include_directories(cpp-qt-operation-parameters
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_compile_definitions(cpp-qt-operation-parameters
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:op-server-go>"
)

target_link_libraries(cpp-qt-operation-parameters PRIVATE OperationParametersClient CommonOperationParametersClient)
add_dependencies(cpp-qt-operation-parameters op-server-go)
