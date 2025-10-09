# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

set(GO_BINARY responses-server-app)
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
add_custom_target(responses-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(responses-server-go IMPORTED)
add_dependencies(responses-server-go responses-server-build)
set_target_properties(responses-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/responses-server-go"
    IMPORTED_GLOBAL TRUE
)

add_custom_command(
    TARGET responses-server-build
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${GO_BINARY}
            ${CMAKE_CURRENT_BINARY_DIR}/responses-server-go
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
            ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMENT "Installing ${GO_BINARY} to ${CMAKE_CURRENT_BINARY_DIR}"
)
