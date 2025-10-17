# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(_qt_internal_openapi_detect_target_type target
    out_is_shared out_is_static out_is_executable)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "SHARED_LIBRARY"
            OR target_type STREQUAL "MODULE_LIBRARY")
        set(is_shared TRUE)
    elseif(target_type STREQUAL "STATIC_LIBRARY")
        set(is_static TRUE)
    elseif(target_type STREQUAL "EXECUTABLE")
        set(is_executable TRUE)
    else()
        message(FATAL_ERROR "Unsupported target type '${target_type}'.")
    endif()

    set(${out_is_shared} "${is_shared}" PARENT_SCOPE)
    set(${out_is_static} "${is_static}" PARENT_SCOPE)
    set(${out_is_executable} "${is_executable}" PARENT_SCOPE)
endfunction()

function(qt6_add_openapi_client target)
    set(options
        GENERATE_COMMON_LIBRARY
        COMPRESSION_REQUIRED
    )
    set(oneValueArgs
        SPEC_FILE
        CPP_NAMESPACE
        MODEL_NAME_PREFIX
        COMMON_LIBRARY_NAME
        OUTPUT_DIRECTORY
    )
    set(multiValueArgs "")
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    if(NOT arg_SPEC_FILE)
        message(FATAL_ERROR "qt6_add_openapi_client: SPEC_FILE is required.")
    endif()

    if(NOT arg_OUTPUT_DIRECTORY)
        set(arg_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if (NOT TARGET "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar")
        message(FATAL_ERROR
            "Can't find ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar target, "
            "please report an issue on the Qt bugtracker")
        return()
    endif()

    get_target_property(openapi_generator_cli_jar_file
        WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE_OPENAPI_GENERATOR_CLI_JAR)

    if(NOT openapi_generator_cli_jar_file)
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "Java OpenAPI generator is not installed or not added in PATH.")
    endif()

    file(MAKE_DIRECTORY "${arg_OUTPUT_DIRECTORY}")
    set(openapi_cli_entrypoint_class "org.openapitools.codegen.OpenAPIGenerator")
    set(generator_name ${QT_OPENAPI_GENERATOR_NAME})
    get_filename_component(openapi_generator_cli_dir
        "${openapi_generator_cli_jar_file}" DIRECTORY)
    get_target_property(generator_path
        "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" IMPORTED_LOCATION)

    # The modelNamePrefix affects names of generated files.
    # The default prefix is defined in CppQt6AbstractCodegen.java:
    # PREFIX = "QtOAI"
    if(NOT arg_MODEL_NAME_PREFIX)
        set(model_name_prefix "QtOAI")
    else()
        set(model_name_prefix "${arg_MODEL_NAME_PREFIX}")
    endif()

    # The default namespace is defined in CppQt6AbstractCodegen.java:
    # cppNamespace = "QtOpenAPI"
    if(NOT arg_CPP_NAMESPACE)
        set(cpp_namespace "QtOpenAPI")
    else()
        set(cpp_namespace "${arg_CPP_NAMESPACE}")
    endif()

    # The default commonLibGenerationType is defined in CppQt6ClientGenerator.java:
    # String commonLibrary = GENERATION_TYPE.COMMON_LIB.value;
    if(arg_GENERATE_COMMON_LIBRARY)
        set(common_lib_generation_type "Use-Common-Lib")
    else()
        set(common_lib_generation_type "Skip-Common-Files")
    endif()

    if(arg_GENERATE_COMMON_LIBRARY AND NOT arg_COMMON_LIBRARY_NAME)
        message(FATAL_ERROR "Please, set common library name via COMMON_LIBRARY_NAME argument.")
    endif()
    set(common_lib_target "${arg_COMMON_LIBRARY_NAME}")

    # ZLIB is used for copression, by default compression is false
    set(compression_required "false")
    if(arg_COMPRESSION_REQUIRED)
        set(compression_required "true")
    endif()

    string(JOIN "," additional_properties
        "--additional-properties=useCmakeMacro=true"
        "cppNamespace=${cpp_namespace}"
        "modelNamePrefix=${model_name_prefix}"
        "commonLibrary=${common_lib_generation_type}"
        "commonLibraryName=${common_lib_target}"
        "contentCompression=${compression_required}"
    )

    if(compression_required)
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR
                "Client generation requires compression support, but the ZLIB::ZLIB "
                "target was not found. Please add find_package(ZLIB) to the top "
                "of your project CMakeLists.txt and ensure the headers and library "
                "can be found by CMake."
                )
        endif()
    endif()

    set(generating_sources "")
    set(common_sources "")
    set(client_sources "")
    set(common_dir "common")
    set(client_dir "client")
    set(client_dir_path "${arg_OUTPUT_DIRECTORY}/${client_dir}/")
    set(common_dir_path "${arg_OUTPUT_DIRECTORY}/${common_dir}/")
    list(APPEND client_sources "${client_dir_path}${model_name_prefix}CombinedModelsAndAPIs.cpp")
    list(APPEND client_sources "${client_dir_path}${model_name_prefix}Exports.h")
    list(APPEND generating_sources ${client_sources})
    if(arg_GENERATE_COMMON_LIBRARY)
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}CommonExports.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}BaseApi.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}BaseApi.cpp")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}Helpers.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}Helpers.cpp")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}HttpRequest.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}HttpRequest.cpp")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}HttpFileElement.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}HttpFileElement.cpp")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}Object.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}Enum.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}ServerConfiguration.h")
        list(APPEND common_sources "${common_dir_path}${model_name_prefix}ServerVariable.h")
        list(APPEND generating_sources ${common_sources})
    endif()

    if(CMAKE_HOST_WIN32)
        set(path_separator "\\;")
    else()
        set(path_separator ":")
    endif()

    set(run_client_cmd
        "${openapi_generator_cli_dir}${path_separator}${openapi_generator_cli_jar_file}${path_separator}${generator_path}")

    set(extra_dependencies "")
    if(TARGET QtOpenAPIGenerator)
        # otherwise add_custom_command tries to run generator, that is not created yet
        set(extra_dependencies DEPENDS QtOpenAPIGenerator)
    endif()

    # if target is already created, no need to use package_name option for generator
    if(TARGET ${target})
        set(target_lib_name "")
    else()
        set(target_lib_name "--package-name=${target}")
    endif()

    add_custom_command(
        OUTPUT ${generating_sources}
        COMMAND java -cp
            "${run_client_cmd}"
            ${openapi_cli_entrypoint_class}
            generate -g ${generator_name}
            -i "${arg_SPEC_FILE}"
            -o "${arg_OUTPUT_DIRECTORY}"
            ${target_lib_name}
            ${additional_properties}
        ${extra_dependencies}
        COMMENT "Generating the Qt6 Client code with the generator: ${generator_name}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)
    if(NOT TARGET ${target})
        qt_add_library(${target})
        set_property(TARGET ${target} PROPERTY AUTOMOC "ON")
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::Core
            ${QT_CMAKE_EXPORT_NAMESPACE}::Network
        )
        if(arg_GENERATE_COMMON_LIBRARY)
            qt_add_library(${common_lib_target})
            set_property(TARGET ${common_lib_target} PROPERTY AUTOMOC "ON")
            target_link_libraries(${common_lib_target} PRIVATE
                ${QT_CMAKE_EXPORT_NAMESPACE}::Core
                ${QT_CMAKE_EXPORT_NAMESPACE}::Network
            )
            if(arg_COMPRESSION_REQUIRED)
                target_link_libraries(${common_lib_target} PRIVATE ZLIB::ZLIB)
            endif()
            target_link_libraries(${target} PRIVATE ${common_lib_target})
        endif()
    endif()

    _qt_internal_openapi_detect_target_type(${target}
        is_shared is_static is_executable)

    if(is_shared)
        target_compile_definitions(${target} PRIVATE
            ${model_name_prefix}_LIB_SHARED)
    elseif(is_static OR is_executable)
        target_compile_definitions(${target} PRIVATE
            ${model_name_prefix}_LIB_STATIC)
    endif()
    if(NOT is_executable)
        target_compile_definitions(${target} PRIVATE
            ${model_name_prefix}_BUILD_LIB)
    endif()

    if(arg_GENERATE_COMMON_LIBRARY)
        _qt_internal_openapi_detect_target_type(${common_lib_target}
            is_shared is_static is_executable)
        if(is_shared)
            target_compile_definitions(${common_lib_target} PRIVATE
                ${model_name_prefix}_COMMON_LIB_SHARED)
        elseif(is_static OR is_executable)
            target_compile_definitions(${common_lib_target} PRIVATE
                ${model_name_prefix}_COMMON_LIB_STATIC)
        endif()
        if(NOT is_executable)
            target_compile_definitions(${common_lib_target} PRIVATE
                ${model_name_prefix}_BUILD_COMMON_LIB)
        endif()
    endif()

    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}>")
    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${client_dir}>")
    if(arg_GENERATE_COMMON_LIBRARY)
        target_include_directories(${target} PUBLIC
            "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${common_dir}>")
    endif()

    if(TARGET QtOpenAPIGenerator)
        add_dependencies(${target} QtOpenAPIGenerator)
    endif()

    target_sources(${target} PRIVATE ${client_sources})
    if(arg_GENERATE_COMMON_LIBRARY)
        target_sources(${common_lib_target} PRIVATE ${common_sources})
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_openapi_client target)
         qt6_add_openapi_client(${ARGN})
    endfunction()
endif()
