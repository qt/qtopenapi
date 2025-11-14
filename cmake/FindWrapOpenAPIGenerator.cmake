# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
    set(WrapOpenAPIGenerator_FOUND TRUE)
    return()
endif()

# We don't use find_jar, because that passes NO_DEFAULT_PATH to find_path, and we want to search
# inside PATH env.
find_file(OPENAPI_GENERATOR_CLI_JAR
    NAMES
        openapi-generator-cli.jar
    PATHS
        # These are the paths that find_jar would search by default
        /usr/share/java
        /usr/local/share/java
        ${Java_JAR_PATHS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapOpenAPIGenerator
    REQUIRED_VARS
        OPENAPI_GENERATOR_CLI_JAR
)

if(WrapOpenAPIGenerator_FOUND)
    add_library(WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE IMPORTED)
    set_target_properties(WrapOpenAPIGenerator::WrapOpenAPIGenerator PROPERTIES
        INTERFACE_OPENAPI_GENERATOR_CLI_JAR "${OPENAPI_GENERATOR_CLI_JAR}"
    )
endif()

