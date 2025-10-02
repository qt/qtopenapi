# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(cpp-qt-operation-parameters)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Test)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_standard_project_setup(REQUIRES 6.8)
add_executable(cpp-qt-operation-parameters
    OperationParameters.cpp
)

target_include_directories(cpp-qt-operation-parameters
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_link_libraries(cpp-qt-operation-parameters PRIVATE OperationParametersClient Qt::Core Qt::Network Qt::Gui Qt::Test)
enable_testing()
add_test(NAME cpp-qt-operation-parameters-test COMMAND cpp-qt-operation-parameters)
