#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

set -ex

function usage() {
    cat <<EOF >&2
Usage: generate [cgl]

The script expects Java environment to be set, and uses openapi-generator-cli.jar
to perform the actual code generation.

**************************
*** One option mode: *****
**************************
'cg' Compile the generator and generate the client code
'doc' Compile the generator, generate the client code and generate doxygen documentation for the client
'l' List options provided by the generator
'qmlcg' Compile the generator with Qml-enabled option set, and generate the Qml-enabled client code
'qmldoc' Compile the generator with Qml-enabled option set, generate the Qml-enabled client code and generate doxygen documentation for the client
'qmltest' Compile the generator with Qml-enabled option set, and run generated code with Qml test application
'test' Compile the generator and run generated code with C++ test application

Example:
./generate.sh cg

**************************
*** Two options mode: ****
**************************
'a/cg/doc/l/qmlcg/qmldoc/qmltest/test' 'specification' Run previously mentioned options for the concrete specification. By default, 'petstore' specification is used.

Example:
./generate.sh cg colorpalette

**************************
*** Three options mode: ****
**************************
'a/cg/doc/l/qmlcg/qmldoc/qmltest/test' 'specification' 'loglevel' Run previously mentioned options with both a specific OpenAPI spec and a custom log level. By default, 'info' log level is used.

Example:
./generate.sh cg colorpalette warn

EOF
    die "$@"
}

function warn() {
    RED='\033[1;31m'
    NC='\033[0m'
    echo -e "${RED}$@${NC}" >&2
}

function die() {
    warn "$@"
    exit 1
}

function mvn_exists() {
    mvn -v "$1" >/dev/null 2>&1
}

MODE="$1"
OPENAPI_CLI="openapi_client_generators/openapi-generator-cli-7.15.0.jar"
OPENAPI_CLI_ENTRYPOINT_CLASS="org.openapitools.codegen.OpenAPIGenerator"
ORIGINAL_GENERATOR="cpp-qt6-client"
ORIGINAL_GENERATOR_JAR="$PWD/target/cpp-qt6-client-openapi-generator-1.0.0.jar"
QML_ADDITIONAL_PROPERTIES=false
PREFIX_NAME=QtOAI
CPP_NAMESPACE=QtOpenAPI
CLIENT_PACKAGE_NAME=ClientName
if [[ $MODE == "qmltest" ]] || [[ $MODE == "qmldoc" ]] || [[ $MODE == "qmlcg" ]]; then
    QML_ADDITIONAL_PROPERTIES=true
    CLIENTFOLDER_NAME=qmlclient
    PREFIX_NAME=OAI
    CPP_NAMESPACE=OpenAPI
else
    CLIENTFOLDER_NAME=client
fi
PROJECT_ROOT=$PWD

function openapi_generator_download() {
    #### Download openapi installation
    if [[ ! -f "$OPENAPI_CLI" ]]; then
        mkdir -p $PWD/openapi_client_generators

        #### Here version should be updated manually
        wget https://repo1.maven.org/maven2/org/openapitools/openapi-generator-cli/7.15.0/openapi-generator-cli-7.15.0.jar -O $PWD/openapi_client_generators/openapi-generator-cli-7.15.0.jar
        export PATH=$PATH:$PWD/openapi_client_generators
    fi
    #### Check downloads
    [[ -f "$OPENAPI_CLI" ]] || usage "Error: openapi-generator-cli.jar does not exist: " \""$OPENAPI_CLI"\"
}

function compile() {
    if mvn_exists bash; then
        echo 'Running "mvn package" command.'
    else
        echo 'Your system does not have mvn. Please, try "sudo apt install maven -y" or set "JAVA_HOME" variable.'
    fi
    # download openapi generator
    openapi_generator_download ;
    # Clean previous build result
    mvn clean
    # Compile
    mvn package
}

function set_paths() {
    # Validate the value
    if [[ -f "$PWD/yaml_files/$USER_MODE.yaml" ]]; then
        USER_SPEC="$PWD/yaml_files/$USER_MODE.yaml"
        SERVER_OUTPUT_DIR="$PWD/tests/auto/$USER_MODE/server"
        CLIENT_OUTPUT_DIR="$PWD/tests/auto/$USER_MODE/$CLIENTFOLDER_NAME"
        if [[ $USER_MODE == "petstore" ]]; then
            if [[ $QML_ADDITIONAL_PROPERTIES == true ]]; then
                CLIENT_PACKAGE_NAME=PetStoreClientQml
            else
                CLIENT_PACKAGE_NAME=PetStoreClient
            fi
            SERVER_NAME="petstore-server-app"
        elif [[ $USER_MODE == "operation-parameters" ]]; then
            CLIENT_PACKAGE_NAME=OperationParametersClient
            SERVER_NAME="server-app"
        elif [[ $USER_MODE == "openapi2.0" ]]; then
            CLIENT_PACKAGE_NAME=OpenapiBackportClient
            SERVER_NAME="backport-server-app"
        elif [[ $USER_MODE == "mediatype" ]]; then
            CLIENT_PACKAGE_NAME=MediaTypeClient
            SERVER_NAME="mediatype-server-app"
        elif [[ $USER_MODE == "colorpalette" ]]; then
            if [[ $QML_ADDITIONAL_PROPERTIES == true ]]; then
                CLIENT_PACKAGE_NAME=ColorpaletteClientQml
            else
                CLIENT_PACKAGE_NAME=ColorpaletteClient
            fi
        fi
    else
        echo "Available specifications in $PWD/yaml_files:"
        ls yaml_files/*.yaml 2>/dev/null | xargs -n1 basename | sed 's/^/  /' >&2
        die "Error: user-spec does not exist."
    fi
}

USER_MODE="$2"
if [[ -z "$USER_MODE" ]]; then
    USER_MODE="petstore" # default
fi

# Set paths based on user_mode
set_paths

# Check for invalid mode vs user_mode combinations
if [[ $MODE == "qmltest" || $MODE == "qmldoc" ]]; then
    if [[ $USER_MODE != "petstore" && $USER_MODE != "colorpalette" ]]; then # only those 2 have qml clients
        die "Skipping: $MODE is not applicable for $USER_MODE."
    fi
fi

# Choose your log level: debug, info, warn, or error
LOG_LEVEL=${3:-INFO}  # Default to INFO if not provided
LOGBACK_XML_PATH="$PWD/logback.xml"
shopt -s nocasematch # make case insensitive
if [[  ${3} != ""  && ${3} != "debug" && ${3} != "info" && ${3} != "warn" && ${3} != "error" ]]; then
    die "The log level \"$3\" is not recognized. Please, use: debug, info, warn, or error."
fi

function generator_exists() {
    if [ ! -e "$ORIGINAL_GENERATOR_JAR" ]; then
        die "File $ORIGINAL_GENERATOR_JAR doesn't exist, please run './generator.sh cg'"
    fi
}

function generate() {
    # Generate the source files by using the custom generator.
    # The specification is the interface description of the user (HTTP API).
    # The config defines which templates and options we customize;
    # this is needed because this PoC implementation doesn't define its
    # own generator, but customizes the pre-existing cpp-qt-client generator
    generator_exists ;
    echo "Generating code with spec $USER_SPEC. Output directory: $CLIENT_OUTPUT_DIR"
    echo "Cleaning up old generated files in output directory..."
    rm -rf $CLIENT_OUTPUT_DIR/client

    java -Dlogback.configurationFile=$LOGBACK_XML_PATH -Dlog.level=$LOG_LEVEL -Dcolor=true \
    -cp $PWD:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS \
    generate -g $ORIGINAL_GENERATOR -i $USER_SPEC -o $CLIENT_OUTPUT_DIR \
    --additional-properties=enableQmlCode=$QML_ADDITIONAL_PROPERTIES,cppNamespace=$CPP_NAMESPACE,modelNamePrefix=$PREFIX_NAME --package-name=$CLIENT_PACKAGE_NAME
}

function killServer() {
    # when the client finished testing, let's kill server ]:->
    exit_pid=$(pidof $SERVER_NAME) || true # safe Pattern (avoid script exit on "not found")
    if [[ $exit_pid != "" ]]; then
        echo "Now kill the server by pid:" $exit_pid
        kill -9 $exit_pid
    fi
}

function run_test() {
    if [[ $USER_MODE == "colorpalette" ]]; then
        #build generated code
        cd $CLIENT_OUTPUT_DIR
        rm -rf $CLIENT_OUTPUT_DIR/build
        source build-and-test.bash
    else
        # build and run server app
        cd $SERVER_OUTPUT_DIR
        rm -rf $SERVER_OUTPUT_DIR/build
        source build-and-run.bash

        #build and run client test apps
        cd $CLIENT_OUTPUT_DIR/
        rm -rf $CLIENT_OUTPUT_DIR/build
        source build-and-test.bash

        # when the client finished testing, let's kill the server ]:->
        killServer
    fi
    cd $PROJECT_ROOT
}

function doxygen_compile() {
    cd $CLIENT_OUTPUT_DIR/client
    rm -rf doc/html doc/latex
    doxygen doc/Doxyfile.in
}

function list() {
    generator_exists ;
    java -classpath $PWD:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS config-help -g $ORIGINAL_GENERATOR
}

# usefull to re-genarate all clients by 1 command
function run_all() {
    QML_ADDITIONAL_PROPERTIES=false
    CLIENTFOLDER_NAME=client
    USER_MODE="petstore"
    PREFIX_NAME=QtOAI
    CPP_NAMESPACE=QtOpenAPI
    CLIENT_PACKAGE_NAME=PetStoreClient
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=true
    CLIENTFOLDER_NAME=qmlclient
    PREFIX_NAME=OAI
    CPP_NAMESPACE=OpenAPI
    CLIENT_PACKAGE_NAME=PetStoreClientQml
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=false
    CLIENTFOLDER_NAME=client
    USER_MODE="colorpalette"
    PREFIX_NAME=QtOAI
    CPP_NAMESPACE=QtOpenAPI
    CLIENT_PACKAGE_NAME=ColorpaletteClient
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=true
    CLIENTFOLDER_NAME=qmlclient
    PREFIX_NAME=OAI
    CPP_NAMESPACE=OpenAPI
    CLIENT_PACKAGE_NAME=ColorpaletteClientQml
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=false
    CLIENTFOLDER_NAME=client
    USER_MODE="operation-parameters"
    PREFIX_NAME=QtOAI
    CPP_NAMESPACE=QtOpenAPI
    CLIENT_PACKAGE_NAME=OperationParametersClient
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=false
    CLIENTFOLDER_NAME=client
    USER_MODE="openapi2.0"
    PREFIX_NAME=QtOAI
    CPP_NAMESPACE=QtOpenAPI
    CLIENT_PACKAGE_NAME=OpenapiBackportClient
    set_paths && compile && generate && run_test

    QML_ADDITIONAL_PROPERTIES=false
    CLIENTFOLDER_NAME=client
    USER_MODE="mediatype"
    PREFIX_NAME=QtOAI
    CPP_NAMESPACE=QtOpenAPI
    CLIENT_PACKAGE_NAME=MediaTypeClient
    set_paths && compile && generate && run_test
}

#may need to clean up from previous execution
killServer

####################################
### SET THESE VARIABLES MANUALLY ###
####################################
#export CMAKE_PREFIX_PATH=""
if [ ! -d $CMAKE_PREFIX_PATH ];then
    die "Please, set 'CMAKE_PREFIX_PATH' path."
fi
if [[ $CMAKE_PREFIX_PATH == "" ]]; then
    die "\nPlease, export CMAKE_PREFIX_PATH to installed Qt version in generate.sh file!\n"
else
    echo "CMAKE_PREFIX_PATH='$CMAKE_PREFIX_PATH' is exported."
fi
#JAVA_HOME="/usr/lib/jvm/java-21-openjdk-amd64"
if [ ! -d $JAVA_HOME ];then
    die "Please, set 'JAVA_HOME' path."
fi
if [[ $JAVA_HOME == "" ]]; then
   die "'JAVA_HOME' need to be set!\n"
fi

# Ensure Go is installed
if ! command -v go >/dev/null; then
    die "'go' is not installed."
fi

case "$MODE" in
    cg) compile && generate;;
    doc) compile && generate && doxygen_compile ;;
    l) list ;;
    qmlcg) compile && generate;;
    qmldoc) compile && generate && doxygen_compile ;;
    qmltest) compile && generate && run_test ;;
    test) compile && generate && run_test ;;
    all) run_all ;;
    *) usage "Error: mode \"$MODE\" is not recognized." ;;
esac

echo Done
