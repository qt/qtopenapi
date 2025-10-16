# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(OPENAPI_JAVA_EXECUTABLE NAMES java)
find_program(OPENAPI_MAVEN_EXECUTABLE NAMES mvn)

if(NOT QT_FEATURE_openapi_generator)
    return()
endif()

if (NOT OPENAPI_JAVA_EXECUTABLE)
    return()
endif()

if (NOT OPENAPI_MAVEN_EXECUTABLE)
    return()
endif()

set(QT6_OPENAPI_GENERATOR_CLI
    "${CMAKE_CURRENT_BINARY_DIR}/target/cpp-qt6-client-openapi-generator-${QT_OPENAPI_GENERATOR_VERSION}.jar"
)

set(CMD_SUFFIX "")
if(CMAKE_HOST_WIN32)
    set(CMD_SUFFIX ".cmd")
endif()

message(STATUS "Starting build of the Qt6 Client Code Generator with Maven.")
execute_process(
    COMMAND "${OPENAPI_MAVEN_EXECUTABLE}${CMD_SUFFIX}"
            -f "${CMAKE_CURRENT_SOURCE_DIR}/pom.xml" package
            -Dqt.build.dir=${CMAKE_CURRENT_BINARY_DIR}/target
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RESULT_VARIABLE maven_result
    OUTPUT_VARIABLE maven_output
    ERROR_VARIABLE maven_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(maven_result)
    message(FATAL_ERROR "Failed to build the Qt6 Client Code Generator with Maven:
        ${maven_error} ${maven_output}")
else()
    message(STATUS "Finished (re-)building the Qt6 Client Code Generator with Maven at configure time.")
endif()

add_executable(QtOpenAPIGeneratorJar IMPORTED GLOBAL)
add_executable("${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" ALIAS QtOpenAPIGeneratorJar)
set_target_properties(QtOpenAPIGeneratorJar PROPERTIES
    IMPORTED_LOCATION
        "${QT6_OPENAPI_GENERATOR_CLI}"
)

if(INSTALL_LIBEXECDIR)
    set(qt_install_libexec_dir "${INSTALL_LIBEXECDIR}")
elseif(QT6_INSTALL_LIBEXECS)
    set(qt_install_libexec_dir "${QT6_INSTALL_LIBEXECS}")
endif()

qt_install(FILES "${QT6_OPENAPI_GENERATOR_CLI}" DESTINATION "${qt_install_libexec_dir}")
