# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-petstore-storeapi LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET petstore-server-go)
    message(WARNING "No petstore-server-go server in the build.")
    return()
endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_internal_add_test(cpp-qt-petstore-storeapi
    SOURCES
        StoreApiTests.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(PetStoreClient)

target_include_directories(cpp-qt-petstore-storeapi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

target_compile_definitions(cpp-qt-petstore-storeapi
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:petstore-server-go>"
)

target_link_libraries(cpp-qt-petstore-storeapi PRIVATE PetStoreClient CommonPetStoreClient)
add_dependencies(cpp-qt-petstore-storeapi petstore-server-go)
