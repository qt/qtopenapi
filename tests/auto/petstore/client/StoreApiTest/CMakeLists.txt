# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(cpp-qt-petstore-storeapi)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Test)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_standard_project_setup(REQUIRES 6.8)
add_executable(cpp-qt-petstore-storeapi
    StoreApiTests.cpp
)

target_include_directories(cpp-qt-petstore-storeapi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_link_libraries(cpp-qt-petstore-storeapi PRIVATE PetStoreClient Qt::Core Qt::Network Qt::Test)

enable_testing()

add_test(NAME cpp-qt-petstore-storeapi-test COMMAND cpp-qt-petstore-storeapi)
