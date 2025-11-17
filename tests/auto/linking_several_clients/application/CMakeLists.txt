# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-test-config LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network Test)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

## application 1:
## Client1 produces common-lib on generation.
## Client2 links common-lib for common files usage.
qt_internal_add_test(cpp-qt-test-config1-app
    SOURCES
        TestConfiguration1.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

# client1 generation
qt6_add_openapi_client(TestClient1
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/linking_several_clients/client1.yaml
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client1
    GENERATE_COMMON_LIBRARY
    COMMON_LIBRARY_NAME TestCommon
)

# client2 generation
qt6_add_openapi_client(TestClient2
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/linking_several_clients/client2.yaml
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client2
)

target_include_directories(TestClient2
    PUBLIC
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/client1/common>
)

qt_autogen_tools_initial_setup(TestClient1)
qt_autogen_tools_initial_setup(TestClient2)
qt_autogen_tools_initial_setup(TestCommon)

target_link_libraries(TestClient2 PRIVATE TestCommon)
target_link_libraries(cpp-qt-test-config1-app PRIVATE TestClient1 TestClient2 TestCommon)

if(WIN32)
    add_custom_command(TARGET cpp-qt-test-config1-app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestCommon>
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestClient1>
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestClient2>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
