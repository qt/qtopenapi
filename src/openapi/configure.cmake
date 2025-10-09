# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# The variable is ON for macOS, needs to be OFF for Golang search.
if(DEFINED QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH)
    set(_qt_openapi_no_look_in_path_backup "${QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH}")
    set(QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH "OFF")
endif()

qt_find_package(WrapOpenAPIGenerator PROVIDED_TARGETS WrapOpenAPIGenerator::WrapOpenAPIGenerator)

if(DEFINED _qt_openapi_no_look_in_path_backup)
    set(QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH ${_qt_openapi_no_look_in_path_backup})
    unset(_qt_openapi_no_look_in_path_backup)
endif()

qt_feature("openapi_generator" PRIVATE
    SECTION "Qt OpenAPI tools"
    LABEL "OpenAPI Generator"
    CONDITION WrapOpenAPIGenerator_FOUND
)

qt_configure_add_summary_section(NAME "Qt OpenAPI tools")
qt_configure_add_summary_entry(ARGS "openapi_generator")
qt_configure_end_summary_section()

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The dependency on Java is not met. Skipping qtopenapi build."
    CONDITION NOT OPENAPI_JAVA_EXECUTABLE
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The dependency on Golang is not met. Skipping qtopenapi build."
    CONDITION NOT OPENAPI_GO_EXECUTABLE
)
