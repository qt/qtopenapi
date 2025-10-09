# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

set(GO_BINARY petstore-server-app)
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
add_custom_target(petstore-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(petstore-server-go IMPORTED)
add_dependencies(petstore-server-go petstore-server-build)
set_target_properties(petstore-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/petstore-server-go"
    IMPORTED_GLOBAL TRUE
)

add_custom_command(
    TARGET petstore-server-build
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${GO_BINARY}
            ${CMAKE_CURRENT_BINARY_DIR}/petstore-server-go
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/qt-logo.png
            ${CMAKE_CURRENT_BINARY_DIR}/qt-logo.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/response.json
            ${CMAKE_CURRENT_BINARY_DIR}/response.json
    COMMENT "Installing ${GO_BINARY} to ${CMAKE_CURRENT_BINARY_DIR}"
)
