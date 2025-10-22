# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_openapi_generator)
    add_subdirectory(colorpalette)
    add_subdirectory(compression)
    add_subdirectory(linking_several_clients)
    add_subdirectory(mediatype)
    add_subdirectory(openapi2.0)
    add_subdirectory(operation-parameters)
    add_subdirectory(petstore)
    add_subdirectory(responses)
endif()
