# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## OpenapiPrivate Module:
#####################################################################

qt_internal_add_module(OpenapiPrivate
    INTERNAL_MODULE
    NO_ADDITIONAL_TARGET_INFO
    NO_GENERATE_METATYPES
    SOURCES
        dummy.cpp
    PUBLIC_LIBRARIES
        Qt::Core
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}OpenapiPrivateExtraTargets.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}OpenApiGeneratorMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}OpenapiPrivateExtraTargets.cmake"
        "${QT_CMAKE_EXPORT_NAMESPACE}OpenApiGeneratorMacros.cmake"
)
set_target_properties(OpenapiPrivate PROPERTIES AUTOMOC OFF)

qt_record_extra_third_party_dependency(OpenapiPrivate WrapOpenAPIJava::WrapOpenAPIJava)
qt_record_extra_third_party_dependency(OpenapiPrivate WrapOpenAPIGenerator::WrapOpenAPIGenerator)
