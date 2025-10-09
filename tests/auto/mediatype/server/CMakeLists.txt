# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

set(GO_BINARY mediatype-server-app)
set(GO_SOURCE main.go)

add_custom_command(
    OUTPUT ${GO_BINARY}
    COMMAND go mod tidy
    COMMAND go build -o ${GO_BINARY} ${GO_SOURCE}
    DEPENDS ${GO_SOURCE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating the golang server: ${GO_BINARY}"
    VERBATIM
)

# Custom target that depends on the output binary
add_custom_target(mediatype-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(mediatype-server-go IMPORTED)
add_dependencies(mediatype-server-go mediatype-server-build)
set_target_properties(mediatype-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/mediatype-server-go"
    IMPORTED_GLOBAL TRUE
)

add_custom_command(
    TARGET mediatype-server-build
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${GO_BINARY}
            ${CMAKE_CURRENT_BINARY_DIR}/mediatype-server-go
    COMMENT "Installing ${GO_BINARY} to ${CMAKE_CURRENT_BINARY_DIR}"
)
