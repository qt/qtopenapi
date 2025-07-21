#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

usage() {
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

warn () {
    RED='\033[1;31m'
    NC='\033[0m'
    echo -e "${RED}$@${NC}" >&2
}

die() {
    warn "$@"
    exit 1
}

mvn_exists()
{
   mvn -v "$1" >/dev/null 2>&1
}

MODE="$1"
CLASSDIR="src/main/java/com/qt/company/codegen/"
OPENAPI_CLI="openapi_client_generators/openapi-generator-cli-7.12.0.jar"
OPENAPI_CLI_ENTRYPOINT_CLASS="org.openapitools.codegen.OpenAPIGenerator"
ORIGINAL_GENERATOR="cpp-qt6-client"
ORIGINAL_GENERATOR_JAR="$PWD/target/cpp-qt6-client-openapi-generator-1.0.0.jar"
QML_ADDITIONAL_PROPERTIES=false
if [[ $MODE == "qmltest" ]] || [[ $MODE == "qmldoc" ]] || [[ $MODE == "qmlcg" ]];then
  QML_ADDITIONAL_PROPERTIES=true
  CLIENTFOLDER_NAME=qmlclient
else
  CLIENTFOLDER_NAME=client
fi

function openapi_generator_download() {
  #### Download openapi installation
  if [[ ! -f "$OPENAPI_CLI" ]]; then
    mkdir -p $PWD/openapi_client_generators
    curl https://raw.githubusercontent.com/OpenAPITools/openapi-generator/master/bin/utils/openapi-generator-cli.sh > $PWD/openapi_client_generators/openapi-generator-cli
    chmod u+x $PWD/openapi_client_generators/openapi-generator-cli

    #### Here version should be updated manually
    wget https://repo1.maven.org/maven2/org/openapitools/openapi-generator-cli/7.12.0/openapi-generator-cli-7.12.0.jar -O $PWD/openapi_client_generators/openapi-generator-cli-7.12.0.jar
    export PATH=$PATH:$PWD/openapi_client_generators
  fi
  #### Check downloads
  [[ -f "$OPENAPI_CLI" ]] || usage "Error: openapi-generator-cli.jar does not exist: " \""$OPENAPI_CLI"\"
  [[ -f "$CLASSDIR/CppQt6ClientGenerator.java" ]] || usage "Error: the java file does not exist in: " "$CLASSDIR"
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

USER_MODE="$2"
if [[ -z "$USER_MODE" ]]; then
    USER_MODE="petstore" # default
fi
# Validate the value
if [[ -f "$PWD/yaml_files/$USER_MODE.yaml" ]]; then
    USER_SPEC="$PWD/yaml_files/$USER_MODE.yaml"
    SERVER_OUTPUT_DIR="$PWD/tests/auto/$USER_MODE/server"
    CLIENT_OUTPUT_DIR="$PWD/tests/auto/$USER_MODE/$CLIENTFOLDER_NAME"
else
    echo "Available specifications in $PWD/yaml_files:"
    ls yaml_files/*.yaml 2>/dev/null | xargs -n1 basename | sed 's/^/  /' >&2 #-l "$PWD/yaml_files"
    die "Error: user-spec does not exist."
fi

if [[ $MODE == "qmltest" && $USER_MODE == "operation-parameters" ]]; then
  echo "Skipping: 'qmltest' is not applicable for 'operation-parameters'."
  exit 1
elif [[ $MODE == "qmldoc" && $USER_MODE == "operation-parameters" ]]; then
  echo "Skipping: 'qmldoc' is not applicable for 'operation-parameters'."
  exit 1
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
        echo "File $ORIGINAL_GENERATOR_JAR doesn't exist, please run './generator.sh cg'"
        exit 1
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
    --additional-properties=enableQmlCode=$QML_ADDITIONAL_PROPERTIES
}

####################################
### SET THE SERVER NAME MANUALLY ###
####################################
if [[ $USER_MODE == "petstore" ]]; then
    SERVER_NAME="cpp-qt-qhttpengine-server"
elif [[ $USER_MODE == "operation-parameters" ]]; then
    SERVER_NAME="server-app"
fi

function killPetServer() {
    # when the client finished testing, let's kill server ]:->
    exit_pid=$(pidof $SERVER_NAME)
    echo "Now kill the server by pid:" $exit_pid
    kill -9 $exit_pid
}

function run_test() {
  if [[ $USER_MODE == "petstore" || $USER_MODE == "operation-parameters" ]]; then
    #may need to clean up from previous execution
    killPetServer
    # build and run server app
    cd $SERVER_OUTPUT_DIR
    rm -rf $SERVER_OUTPUT_DIR/build
    source build-and-run.bash

    #build and run client test apps
    cd $CLIENT_OUTPUT_DIR/
    rm -rf $CLIENT_OUTPUT_DIR/build
    source build-and-test.bash

    # when the client finished testing, let's kill the server ]:->
    killPetServer
  else #colorpalette and others
      #build generated code
      cd $CLIENT_OUTPUT_DIR
      rm -rf $CLIENT_OUTPUT_DIR/build
      # TODO delete here and in .gitignore after colorpalette client app will be added
      rm -rf $CLIENT_OUTPUT_DIR/libQt6OpenAPIClient_module.so
      source build-and-test.bash
  fi
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

####################################
### SET THESE VARIABLES MANUALLY ###
####################################
#export CMAKE_PREFIX_PATH=""
if [ ! -d $CMAKE_PREFIX_PATH ];then
echo "Please, set 'CMAKE_PREFIX_PATH' path."
exit 1
fi
if [[ $CMAKE_PREFIX_PATH == "" ]]; then
    echo -e "\n"
    echo "Please, export CMAKE_PREFIX_PATH to installed Qt version in generate.sh file!"
    echo -e "\n"
    exit 1
else
    echo "CMAKE_PREFIX_PATH='$CMAKE_PREFIX_PATH' is exported."
fi
#JAVA_HOME="/usr/lib/jvm/java-21-openjdk-amd64"
if [ ! -d $JAVA_HOME ];then
echo "Please, set 'JAVA_HOME' path."
exit 1
fi
if [[ $JAVA_HOME == "" ]]; then
   echo "'JAVA_HOME' need to be set!"
   echo -e "\n"
   exit 1
fi

# Ensure Go is installed
if ! command -v go >/dev/null; then
    echo "'go' is not installed."
    exit 1
fi

case "$MODE" in
    cg) compile && generate;;
    doc) compile && generate && doxygen_compile ;;
    l) list ;;
    qmlcg) compile && generate;;
    qmldoc) compile && generate && doxygen_compile ;;
    qmltest) compile && generate && run_test ;;
    test) compile && generate && run_test ;;
    *) usage "Error: mode \"$MODE\" is not recognized." ;;
esac

echo Done
