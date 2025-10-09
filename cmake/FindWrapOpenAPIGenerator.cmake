# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
    set(WrapOpenAPIGenerator_FOUND TRUE)
    return()
endif()

set(WrapOpenAPIGenerator_FOUND FALSE)

find_program(OPENAPI_JAVA_EXECUTABLE java REQUIRED)
find_program(OPENAPI_GO_EXECUTABLE NAMES go REQUIRED)

if (NOT OPENAPI_GO_EXECUTABLE)
    return()
endif()

if(NOT OPENAPI_JAVA_EXECUTABLE)
    return()
endif()

find_file(OPENAPI_GENERATOR_CLI
    NAMES openapi-generator-cli.jar
)

if(OPENAPI_GENERATOR_CLI AND EXISTS "${OPENAPI_GENERATOR_CLI}")
    set(WrapOpenAPIGenerator_FOUND TRUE)
    set(OPENAPI_GENERATOR_CLI_JAR "${OPENAPI_GENERATOR_CLI}")
endif()

# Define IMPORTED INTERFACE target
add_library(WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE IMPORTED)

# Set properties (for documentation; you can extend as needed)
set_target_properties(WrapOpenAPIGenerator::WrapOpenAPIGenerator PROPERTIES
    INTERFACE_OPENAPI_GENERATOR_CLI_JAR "${OPENAPI_GENERATOR_CLI_JAR}"
)
