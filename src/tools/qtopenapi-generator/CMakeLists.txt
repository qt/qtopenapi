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

# Define a custom command to run 'mvn package'
add_custom_command(
    OUTPUT "${QT6_OPENAPI_GENERATOR_CLI}"
    COMMAND "${OPENAPI_MAVEN_EXECUTABLE}"
            -f "${CMAKE_CURRENT_SOURCE_DIR}/pom.xml" package
            -Dqt.build.dir=${CMAKE_CURRENT_BINARY_DIR}/target
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building the Qt6 Client Code Generator with Maven"
    VERBATIM
)

# Define a custom target that depends on the above output
add_custom_target(
    QtOpenAPIGenerator ALL
    DEPENDS
        "${QT6_OPENAPI_GENERATOR_CLI}"
)

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
