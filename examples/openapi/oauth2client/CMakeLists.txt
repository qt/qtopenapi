# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(oauth2client LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

#! [find-package]
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network NetworkAuth OpenApi)
#! [find-package]

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(oauth2client
    main.cpp
    profileclient.h
    profileclient.cpp
)

#! [generate-client]
qt_add_library(UserProfileApi)
qt_add_openapi_client(UserProfileApi
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/oauth2client.yaml
)
#! [generate-client]

#! [link]
target_link_libraries(oauth2client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
    UserProfileApi
)
#! [link]

# The demo OAuth 2.0 server, built from the same specification with Go.
add_subdirectory(server)
if(TARGET oauth2client-server-go)
    add_dependencies(oauth2client oauth2client-server-go)
    target_compile_definitions(oauth2client PRIVATE
        SERVER_PATH="$<TARGET_FILE:oauth2client-server-go>"
        SERVER_FOLDER="$<TARGET_FILE_DIR:oauth2client-server-go>"
    )
endif()

install(TARGETS oauth2client UserProfileApi
    BUNDLE  DESTINATION "${INSTALL_EXAMPLEDIR}"
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
