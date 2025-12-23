# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(__qt_openapi_macros_module_base_dir "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

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
        GENERATE_DOCUMENTATION
        __QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET
    )
    set(oneValueArgs
        SPEC_FILE
        CPP_NAMESPACE
        CLIENT_PREFIX
        OUTPUT_DIRECTORY
        DOCUMENTATION_OUTPUT_DIRECTORY
        OUTPUT_PUBLIC_HEADERS_DIR
        OUTPUT_PRIVATE_HEADERS_DIR
    )
    set(multiValueArgs "")
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    if(NOT arg_SPEC_FILE)
        message(FATAL_ERROR "qt6_add_openapi_client: SPEC_FILE is required.")
    endif()

    if(NOT arg_OUTPUT_DIRECTORY)
        set(arg_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if(NOT TARGET "${target}")
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "Target '${target}' does not exist. "
            "Please create it before calling qt6_add_openapi_client. "
            "You can use qt_add_library or qt_add_executable to create the target.")
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET AND NOT QT_BUILDING_QT)
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "__QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET is an internal option, and shouldn't be "
            "used outside of the Qt build.")
    endif()

    if (NOT TARGET "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar")
        message(FATAL_ERROR
            "Can't find ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar target, "
            "please report an issue on the Qt bugtracker")
        return()
    endif()

    get_target_property(openapi_generator_cli_jar_file
        WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE_OPENAPI_GENERATOR_CLI_JAR)
    get_target_property(qtcore_comomn_namespace
            ${QT_CMAKE_EXPORT_NAMESPACE}::Core QT_NAMESPACE)

    if(NOT openapi_generator_cli_jar_file)
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "Java OpenAPI generator is not installed or not added in PATH.")
    endif()

    file(MAKE_DIRECTORY "${arg_OUTPUT_DIRECTORY}")
    if(NOT arg_DOCUMENTATION_OUTPUT_DIRECTORY)
        set(arg_DOCUMENTATION_OUTPUT_DIRECTORY "${arg_OUTPUT_DIRECTORY}")
    endif()
    set(openapi_cli_entrypoint_class "org.openapitools.codegen.OpenAPIGenerator")
    set(generator_name ${QT_OPENAPI_GENERATOR_NAME})
    get_filename_component(openapi_generator_cli_dir
        "${openapi_generator_cli_jar_file}" DIRECTORY)
    get_target_property(generator_path
        "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" IMPORTED_LOCATION)

    # The modelNamePrefix affects names of model and api files.
    set(model_and_api_name_prefix "")
    if(arg_CLIENT_PREFIX)
        set(model_and_api_name_prefix "${arg_CLIENT_PREFIX}")
    endif()

    # Qt pre-generated qt common library always uses QOAI
    # prefix, for generating different common library,
    # please call the generator manually.
    set(qt_commonlib_prefix "QOAI")
    set(qt_common_file_prefix "qoai")
    # For qt macro: either QtOpenApiCommon or QtNameSpace::QtOpenApiCommon
    if(qtcore_comomn_namespace)
        set(cpp_common_namespace "${qtcore_comomn_namespace}::QtOpenApiCommon")
    else()
        set(cpp_common_namespace "QtOpenApiCommon")
    endif()

    # The default namespace is defined in CppQt6AbstractCodegen.java:
    # cppNamespace = "QtOpenAPI"
    if(arg_CPP_NAMESPACE)
        set(cpp_namespace "${arg_CPP_NAMESPACE}")
    else()
        set(cpp_namespace "QtOpenAPI")
    endif()

    # The default commonLibGenerationType is defined in CppQt6ClientGenerator.java:
    # String commonLibrary = GENERATION_TYPE.COMMON_LIB.value;
    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(common_lib_generation_type "Gen-Common-Lib")
    else()
        set(common_lib_generation_type "Gen-Client-Lib")
    endif()

    set(common_lib_name "CommonLibrary")

    # The common library always needs compression support.
    set(compression_required "true")

    string(JOIN "," additional_properties
        "--additional-properties=useCmakeMacro=true"
        "packageName=${target}"
        "cppNamespace=${cpp_namespace}"
        "cppCommonNamespace=${cpp_common_namespace}"
        "modelNamePrefix=${model_and_api_name_prefix}"
        "prefix=${qt_commonlib_prefix}"
        "commonLibrary=${common_lib_generation_type}"
        "commonLibraryName=${common_lib_name}"
        "contentCompression=${compression_required}"
    )

    set(generating_sources "")
    set(common_sources "")
    set(client_sources "")
    set(common_dir "common")
    set(client_dir "client")
    set(target_lower_case ${target})
    string(TOLOWER ${target} target_lower_case)
    set(client_dir_path "${arg_OUTPUT_DIRECTORY}/${client_dir}/")
    set(common_dir_path "${arg_OUTPUT_DIRECTORY}/${common_dir}/")
    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        list(APPEND common_sources "${common_dir_path}${target_lower_case}commonexports.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}baseapi.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}baseapi.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}commonglobal.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}helpers.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}helpers.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httprequest.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httprequest.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httpfileelement.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httpfileelement.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}object.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}object.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}enum.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}enum.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}serverconfiguration.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}serverconfiguration.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}servervariable.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}servervariable.cpp")
        list(APPEND generating_sources ${common_sources})
    else()
        list(APPEND client_sources
            "${client_dir_path}${target_lower_case}combinedmodelsandapis.cpp")
        list(APPEND client_sources "${client_dir_path}${target_lower_case}exports.h")
        list(APPEND client_sources "${client_dir_path}doc/Doxyfile.in")
        list(APPEND generating_sources ${client_sources})
    endif()

    if(CMAKE_HOST_WIN32)
        set(path_separator "\\;")
    else()
        set(path_separator ":")
    endif()

    set(run_client_cmd
        "${openapi_generator_cli_dir}${path_separator}${openapi_generator_cli_jar_file}${path_separator}${generator_path}")

    set(extra_dependencies
        DEPENDS
            "${openapi_generator_cli_jar_file}"
            "${generator_path}"
            "${arg_SPEC_FILE}"
    )
    if(TARGET QtOpenAPIGenerator)
        # otherwise add_custom_command tries to run generator, that is not created yet
        list(APPEND extra_dependencies
            QtOpenAPIGenerator
        )
    endif()

    # if target is already created, no need to use package_name option for generator
    if(TARGET ${target})
        set(target_lib_name "")
    else()
        set(target_lib_name "--package-name=${target}")
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(comment "Generating the Qt6 Common Library code with the generator: ${generator_name}")
    else()
        set(comment "Generating the Qt6 Client code with the generator: ${generator_name}")
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
        COMMENT "${comment}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)

    if(NOT arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set_property(TARGET ${target} PROPERTY AUTOMOC "ON")
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::Core
            ${QT_CMAKE_EXPORT_NAMESPACE}::Network
        )

        # This is PUBLIC, because the consumer of the client library
        # needs to get also the common library includes.
        target_link_libraries(${target} PUBLIC Qt6::OpenApiCommon)
    endif()

    _qt_internal_openapi_detect_target_type(${target}
        is_shared is_static is_executable)

    set(define_infix "")
    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(define_infix "_COMMON")
    endif()

    set(target_export ${target})
    string(TOUPPER ${target} target_export)
    if(is_shared)
        target_compile_definitions(${target} PRIVATE
            ${target_export}${define_infix}_LIB_SHARED)
    elseif(is_static OR is_executable)
        target_compile_definitions(${target} PRIVATE
            ${target_export}${define_infix}_LIB_STATIC)
    endif()
    if(NOT is_executable)
        target_compile_definitions(${target} PRIVATE
            ${target_export}_BUILD${define_infix}_LIB)
    endif()

    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}>")
    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${client_dir}>")

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        target_include_directories(${target} PRIVATE
            "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${common_dir}>")
    endif()

    if(TARGET QtOpenAPIGenerator)
        add_dependencies(${target} QtOpenAPIGenerator)
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        target_sources(${target} PRIVATE ${common_sources})
    else()
        target_sources(${target} PRIVATE ${client_sources})
    endif()

    # Generate Doxygen documentation for the Client.
    if(arg_GENERATE_DOCUMENTATION)
        # If Doxygen is not installed, then just skipping.
        if(TARGET Doxygen::doxygen)
            file(MAKE_DIRECTORY "${arg_DOCUMENTATION_OUTPUT_DIRECTORY}")
            set(configure_doxygen_script
            "${__qt_openapi_macros_module_base_dir}/Qt6OpenApiConfigureDoxygenScript.cmake")
            set(MAIN_GEN_DOC_FILE
                "${arg_DOCUMENTATION_OUTPUT_DIRECTORY}/doc/html/index.html")
            set(DOXYGEN_FILE "${client_dir_path}doc/Doxyfile")
            add_custom_command(
                OUTPUT "${MAIN_GEN_DOC_FILE}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DDOXYGEN_IN_FILE_PATH=${DOXYGEN_FILE}.in"
                    "-DDOXYGEN_OUT_FILE_PATH=${DOXYGEN_FILE}"
                    "-DEXCLUDE_FILE=${client_dir_path}${target_lower_case}combinedmodelsandapis.cpp"
                    "-DINPUT_DIR=${client_dir_path}"
                    "-DOUTPUT_DIR=${arg_DOCUMENTATION_OUTPUT_DIRECTORY}/doc"
                    -P "${configure_doxygen_script}"
                COMMAND $<TARGET_FILE:Doxygen::doxygen> "${DOXYGEN_FILE}"
                ${extra_dependencies}
                ${generating_sources}
                COMMENT "Generating doxygen documentation for target '${target}'"
                VERBATIM
                COMMAND_EXPAND_LISTS
            )
            add_custom_target(${target}_openapi_docs ALL
                DEPENDS "${MAIN_GEN_DOC_FILE}"
            )
        else()
            message(WARNING "Could not find doxygen to generate documentation. "
                    "Either install it or make sure it's added to PATH and then call "
                    "find_package(Doxygen) in your project."
                    "Skipping documentation generation.")
        endif()
    endif()

    if(arg_OUTPUT_PUBLIC_HEADERS_DIR OR arg_OUTPUT_PRIVATE_HEADERS_DIR)
        set(client_public_headers_dir "${arg_OUTPUT_DIRECTORY}/${client_dir}_public_headers")
        set(client_private_headers_dir "${arg_OUTPUT_DIRECTORY}/${client_dir}_private_headers")
        set(copied_headers_timestamp_file
            "${arg_OUTPUT_DIRECTORY}/openapi_client_copied_headers_timestamp.txt")
        set(copy_headers_script
            "${__qt_openapi_macros_module_base_dir}/Qt6OpenApiToolsCopyHeadersScript.cmake")

        add_custom_command(
            OUTPUT "${copied_headers_timestamp_file}"
            DEPENDS
                "${copy_headers_script}"
                ${generating_sources}
            COMMAND "${CMAKE_COMMAND}"
                "-DTIMESTAMP_PATH=${copied_headers_timestamp_file}"
                "-DCLIENT_DIR=${client_dir_path}"
                "-DOUTPUT_PUBLIC_HEADERS_DIR=${client_public_headers_dir}"
                "-DOUTPUT_PRIVATE_HEADERS_DIR=${client_private_headers_dir}"
                -P "${copy_headers_script}"
            COMMENT "Copying generated OpenApi headers for target '${target}'"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )
        target_sources(${target} PRIVATE "${copied_headers_timestamp_file}")
    endif()

    if(arg_OUTPUT_PUBLIC_HEADERS_DIR)
        set(${arg_OUTPUT_PUBLIC_HEADERS_DIR} "${client_public_headers_dir}" PARENT_SCOPE)
    endif()
    if(arg_OUTPUT_PRIVATE_HEADERS_DIR)
        set(${arg_OUTPUT_PRIVATE_HEADERS_DIR} "${client_private_headers_dir}" PARENT_SCOPE)
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_openapi_client)
         qt6_add_openapi_client(${ARGV})
    endfunction()
endif()
