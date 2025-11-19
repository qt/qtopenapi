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
    add_compile_options(-Wall -Wextra)
    if(WARNINGS_ARE_ERRORS)
        add_compile_options(-Werror=deprecated-declarations)
    endif()
endif()

find_package(ZLIB REQUIRED)

qt_internal_add_test(cpp-qt-compression
    SOURCES
        tst_compression.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(CompressionClient)
qt_add_library(CommonCompressionClient)
qt6_add_openapi_client(CompressionClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/compression.yaml
    COMMON_LIBRARY_TARGET CommonCompressionClient
    COMPRESSION_REQUIRED
)

# target_compile_definitions(cpp-qt-compression
#     PRIVATE
#         SERVER_PATH="$<TARGET_FILE:compression-server-go>"
# )
qt_autogen_tools_initial_setup(CompressionClient)
qt_autogen_tools_initial_setup(CommonCompressionClient)
target_link_libraries(cpp-qt-compression PRIVATE CompressionClient CommonCompressionClient)
# add_dependencies(cpp-qt-compression compression-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-compression POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CommonCompressionClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CompressionClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
