# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## OpenapiPrivate Module:
#####################################################################

qt_internal_add_module(OpenapiPrivate
    INTERNAL_MODULE
    #HEADER_MODULE
    #NO_GENERATE_CPP_EXPORTS
    #NO_MODULE_HEADERS
    NO_ADDITIONAL_TARGET_INFO
    SOURCES
        dummy.cpp
    PUBLIC_LIBRARIES
        Qt::Core
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}OpenapiPrivateExtraTargets.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}OpenapiPrivateExtraTargets.cmake"
)
