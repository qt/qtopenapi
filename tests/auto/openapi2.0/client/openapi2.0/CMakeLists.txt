# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-operations-parameters-backport LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET backport-server-go)
    message(WARNING "No backport-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-operations-parameters-backport
    SOURCES
        OperationParametersBackport.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(OpenapiBackportClient)

target_include_directories(cpp-qt-operations-parameters-backport
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_compile_definitions(cpp-qt-operations-parameters-backport
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:backport-server-go>"
)

add_dependencies(cpp-qt-operations-parameters-backport backport-server-go)
target_link_libraries(cpp-qt-operations-parameters-backport
    PRIVATE
        OpenapiBackportClient
        CommonOpenapiBackportClient
)

