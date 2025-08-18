#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# maven is required coz Client generator is based on java code
sudo apt-get -y install maven

OPENAPI_HOME=/home/qt/work/playground/qtopenapi
JAVA_HOME=$(dirname $(dirname $(readlink -e /usr/bin/javac)))
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
for i in "${testFolders[@]}"
do
    CLIENTFOLDER_NAME=client
    USER_SPEC="$OPENAPI_HOME/yaml_files/$i.yaml"
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$i/$CLIENTFOLDER_NAME"
    rm -rf CLIENT_OUTPUT_DIR/client
    java -cp $OPENAPI_HOME:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS generate -g $ORIGINAL_GENERATOR -i $USER_SPEC -o $CLIENT_OUTPUT_DIR --additional-properties=enableQmlCode=false,cppNamespace=QtOpenAPI,modelNamePrefix=QtOAI
done

#qml clients
qmlTestFolders=("petstore" "colorpalette")
for i in "${qmlTestFolders[@]}"
do
    USER_SPEC="$OPENAPI_HOME/yaml_files/$i.yaml"
    CLIENTFOLDER_NAME=qmlclient
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$i/$CLIENTFOLDER_NAME"
    rm -rf CLIENT_OUTPUT_DIR/client
    java -cp $OPENAPI_HOME:$OPENAPI_CLI:$ORIGINAL_GENERATOR_JAR $OPENAPI_CLI_ENTRYPOINT_CLASS generate -g $ORIGINAL_GENERATOR -i $USER_SPEC -o $CLIENT_OUTPUT_DIR --additional-properties=enableQmlCode=true,cppNamespace=OpenAPI,modelNamePrefix=OAI
done
