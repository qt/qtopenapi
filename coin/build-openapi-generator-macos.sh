#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# maven is required coz Client generator is based on java code
brew install maven

OPENAPI_HOME=/Users/qt/work/playground/qtopenapi
cd $OPENAPI_HOME

mvn clean
mvn package

OPENAPI_CLI="/opt/qt-openapi/openapi-generator-cli.jar"
OPENAPI_CLI_ENTRYPOINT_CLASS="org.openapitools.codegen.OpenAPIGenerator"
ORIGINAL_GENERATOR="cpp-qt6-client"
ORIGINAL_GENERATOR_JAR="$OPENAPI_HOME/target/cpp-qt6-client-openapi-generator-1.0.0.jar"

if [[ ! -e "$ORIGINAL_GENERATOR_JAR" ]]; then
    echo "File $ORIGINAL_GENERATOR_JAR doesn't exist"
    exit 1
fi

# cpp clients + default namespace and model name
testFolders=("petstore" "operation-parameters" "mediatype" "colorpalette" "openapi2.0")
clientNames=("PetStoreClient" "OperationParametersClient" "MediaTypeClient" "ColorpaletteClient" "OpenapiBackportClient")
length=${#testFolders[@]}
for ((i = 0; i < length; i++)); do
    CLIENTFOLDER_NAME=client
    USER_SPEC="$OPENAPI_HOME/yaml_files/${testFolders[i]}.yaml"
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/${testFolders[i]}/$CLIENTFOLDER_NAME"
    rm -rf CLIENT_OUTPUT_DIR/client
    java -cp $OPENAPI_HOME:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS generate -g $ORIGINAL_GENERATOR -i $USER_SPEC -o $CLIENT_OUTPUT_DIR --additional-properties=enableQmlCode=false,cppNamespace=QtOpenAPI,modelNamePrefix=QtOAI --package-name=${clientNames[i]}
done

#qml clients
qmlTestFolders=("petstore" "colorpalette")
qmlClientNames=("PetStoreClientQml" "ColorpaletteClientQml")
qmlLength=${#qmlTestFolders[@]}
for ((i = 0; i < qmlLength; i++)); do
    USER_SPEC="$OPENAPI_HOME/yaml_files/${qmlTestFolders[i]}.yaml"
    CLIENTFOLDER_NAME=qmlclient
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/${qmlTestFolders[i]}/$CLIENTFOLDER_NAME"
    rm -rf CLIENT_OUTPUT_DIR/client
    java -cp $OPENAPI_HOME:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS generate -g $ORIGINAL_GENERATOR -i $USER_SPEC -o $CLIENT_OUTPUT_DIR --additional-properties=enableQmlCode=true,cppNamespace=OpenAPI,modelNamePrefix=OAI --package-name=${qmlClientNames[i]}
done

