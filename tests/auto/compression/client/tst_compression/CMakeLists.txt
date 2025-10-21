# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-compression LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# We do not need a server now
# if(NOT TARGET compression-server-go)
#     message(WARNING "No compression-server-go server in the build.")
#     return()
# endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror=deprecated-declarations)
endif()

qt_internal_add_test(cpp-qt-compression
    SOURCES
        tst_compression.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_autogen_tools_initial_setup(CompressionClient)

target_include_directories(cpp-qt-compression
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/../client>
)

# target_compile_definitions(cpp-qt-compression
#     PRIVATE
#         SERVER_PATH="$<TARGET_FILE:compression-server-go>"
# )

target_link_libraries(cpp-qt-compression PRIVATE CompressionClient)
# add_dependencies(cpp-qt-compression compression-server-go)
