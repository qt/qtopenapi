# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-unique-items LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(cpp-qt-unique-items
    SOURCES
        tst_uniqueitems.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::OpenApiCommon
        Qt::Test
)

qt_add_library(UniqueItemsTestClient)
qt_add_openapi_client(UniqueItemsTestClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/unique-items.yaml
)

qt_autogen_tools_initial_setup(UniqueItemsTestClient)
# The test mainly checks that a client using array parameters with
# "uniqueItems: true" (generated as QSet) compiles.
target_link_libraries(cpp-qt-unique-items PRIVATE UniqueItemsTestClient)

if(WIN32)
    add_custom_command(TARGET cpp-qt-unique-items POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:UniqueItemsTestClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
