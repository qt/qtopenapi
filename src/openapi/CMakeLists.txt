# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## OpenapiPrivate Module:
#####################################################################

qt_internal_add_module(OpenApi
    HEADER_MODULE
    NO_MODULE_HEADERS
    NO_PRIVATE_MODULE
    NO_GENERATE_CPP_EXPORTS
    NO_ADDITIONAL_TARGET_INFO
    NO_GENERATE_METATYPES
    NO_PACKAGE_CONFIG_FILE
    NO_MODULE_JSON_FILE
    NO_QMAKE_SUPPORT_FILES
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}OpenApiExtraTargets.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}OpenApiGeneratorMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}OpenApiExtraTargets.cmake"
        "${QT_CMAKE_EXPORT_NAMESPACE}OpenApiGeneratorMacros.cmake"
)
set_target_properties(OpenApi PROPERTIES AUTOMOC OFF)

qt_record_extra_third_party_dependency(OpenApi WrapOpenAPIJava::WrapOpenAPIJava)
qt_record_extra_third_party_dependency(OpenApi WrapOpenAPIGenerator::WrapOpenAPIGenerator)
