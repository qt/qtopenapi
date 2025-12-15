# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT Java_JAVAC_EXECUTABLE)
    message(FATAL_ERROR "Missing Java_JAVAC_EXECUTABLE to be set.")
endif()

if(NOT OPENAPI_MAVEN_EXECUTABLE)
     message(FATAL_ERROR "Missing OPENAPI_MAVEN_EXECUTABLE to be set.")
 endif()

set(QT6_OPENAPI_GENERATOR_CLI
    "${CMAKE_CURRENT_BINARY_DIR}/target/cpp-qt6-client-openapi-generator-${QT_OPENAPI_GENERATOR_VERSION}.jar"
)

set(source_files
    src/main/java/org/qtproject/qt/codegen/CppQt6AbstractCodegen.java
    src/main/java/org/qtproject/qt/codegen/CppQt6ClientGenerator.java
    src/main/resources/META-INF/services/org.openapitools.codegen.CodegenConfig
    src/main/resources/cpp-qt6-client/CMakeConfig.mustache
    src/main/resources/cpp-qt6-client/CMakeLists.txt.mustache
    src/main/resources/cpp-qt6-client/README.mustache
    src/main/resources/cpp-qt6-client/api-body.mustache
    src/main/resources/cpp-qt6-client/api-header.mustache
    src/main/resources/cpp-qt6-client/common/CMakeConfig.mustache
    src/main/resources/cpp-qt6-client/common/CMakeLists.txt.mustache
    src/main/resources/cpp-qt6-client/common/httpfileelement.cpp.mustache
    src/main/resources/cpp-qt6-client/common/httpfileelement.h.mustache
    src/main/resources/cpp-qt6-client/common/httprequest.cpp.mustache
    src/main/resources/cpp-qt6-client/common/httprequest.h.mustache
    src/main/resources/cpp-qt6-client/common/serverconfiguration.cpp.mustache
    src/main/resources/cpp-qt6-client/common/serverconfiguration.h.mustache
    src/main/resources/cpp-qt6-client/common/servervariable.cpp.mustache
    src/main/resources/cpp-qt6-client/common/servervariable.h.mustache
    src/main/resources/cpp-qt6-client/common/api-base-body.mustache
    src/main/resources/cpp-qt6-client/common/api-base-header.mustache
    src/main/resources/cpp-qt6-client/common/common-exports.mustache
    src/main/resources/cpp-qt6-client/common/enum-body.mustache
    src/main/resources/cpp-qt6-client/common/enum-header.mustache
    src/main/resources/cpp-qt6-client/common/helpers-body.mustache
    src/main/resources/cpp-qt6-client/common/helpers-header.mustache
    src/main/resources/cpp-qt6-client/common/object-body.mustache
    src/main/resources/cpp-qt6-client/common/object-header.mustache
    src/main/resources/cpp-qt6-client/doc/Doxyfile.in.mustache
    src/main/resources/cpp-qt6-client/exports.mustache
    src/main/resources/cpp-qt6-client/licenseInfo.mustache
    src/main/resources/cpp-qt6-client/model-body.mustache
    src/main/resources/cpp-qt6-client/model-header.mustache
    src/test/java/org/qtproject/qt/codegen/CppQt6ClientGeneratorTest.java
    pom.xml
    logback.xml
)


# Define a custom command to run 'mvn package'
add_custom_command(
    OUTPUT "${QT6_OPENAPI_GENERATOR_CLI}"
    COMMAND "${OPENAPI_MAVEN_EXECUTABLE}"
            -f "${CMAKE_CURRENT_SOURCE_DIR}/pom.xml" package
            -Dqt.build.dir=${CMAKE_CURRENT_BINARY_DIR}/target
    # Copy the built generator to libexec build dir, so that standalone tests work with no-prefix
    # builds as well.
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${QT6_OPENAPI_GENERATOR_CLI}"
            "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}"
    DEPENDS
        ${source_files}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building the Qt6 Client Code Generator with Maven"
    VERBATIM
    USES_TERMINAL
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
