# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

# Builds the demo authorization and resource server that the oauth2client
# example talks to. Its /api routes are generated from ../oauth2client.yaml with
# the go-gin-server generator of the upstream OpenAPI Generator, the OAuth 2.0
# endpoints are hand-written in go/oauth_server.go.

cmake_minimum_required(VERSION 3.16)

find_program(OAUTH2CLIENT_GO_EXECUTABLE NAMES go)

if(NOT OAUTH2CLIENT_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH. "
        "The oauth2client example will not start its demo server itself; "
        "run 'go run .' in ${CMAKE_CURRENT_SOURCE_DIR} before starting the example.")
    return()
endif()

set(GO_BINARY oauth2client-server-app)
file(GLOB GO_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.go
    ${CMAKE_CURRENT_SOURCE_DIR}/go/*.go
    ${CMAKE_CURRENT_SOURCE_DIR}/go.mod
)

add_custom_command(
    OUTPUT ${GO_BINARY}
    COMMAND ${OAUTH2CLIENT_GO_EXECUTABLE} mod tidy
    COMMAND ${OAUTH2CLIENT_GO_EXECUTABLE} build -buildvcs=false
            -o "${CMAKE_CURRENT_BINARY_DIR}/${GO_BINARY}" .
    DEPENDS ${GO_SOURCES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building the demo OAuth 2.0 server: ${GO_BINARY}"
    VERBATIM
)

add_custom_target(oauth2client-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(oauth2client-server-go IMPORTED GLOBAL)
add_dependencies(oauth2client-server-go oauth2client-server-build)
set_target_properties(oauth2client-server-go PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${GO_BINARY}"
)
