#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# petstore test server uses the old Qt5 libs for the server implementation
# TBD QTBUG-137879: remove dependency to the old Qt5 libs, and refactore current server or implement a new one
sudo apt-get -y install qtbase5-dev qtbase5-dev-tools
# Qt6 generator uses doxygen to provide documentation to the user
sudo apt-get -y install doxygen graphviz
# operation-parameters test server uses Go
sudo apt-get -y install golang
# Fetch missing Go dependencies (only works if run from within a Go module)
if [[ -f "go.mod" ]]; then
    go mod tidy
fi
OPENAPI_HOME=/home/qt/work/playground/qtopenapi
export CMAKE_PREFIX_PATH="/home/qt/work/install"

function run_test() {
    #build and run client test apps
    cd $CLIENT_OUTPUT_DIR/
    rm -rf $CLIENT_OUTPUT_DIR/build
    source build-and-test.bash
}

function run_test_server() {
    SERVER_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$1/server"
    if [[ $1 == "petstore" ]]; then
        SERVER_NAME="cpp-qt-qhttpengine-server"
        rm -rf $SERVER_OUTPUT_DIR/build
    elif [[ $1 == "colorpalette" ]]; then
        SERVER_NAME=""
        return # no server yet
    elif [[ $1 == "operation-parameters" ]]; then
        SERVER_NAME="server-app"
    elif [[ $1 == "openapi2.0" ]]; then
        SERVER_NAME="backport-server-app"
    elif [[ $1 == "mediatype" ]]; then
        SERVER_NAME="mediatype-server-app"
    fi
    cd "$SERVER_OUTPUT_DIR"
    source build-and-run.bash
}

function kill_test_server() {
    # when the client finished testing, let's kill server ]:->
    if [[ $SERVER_NAME == "" ]]; then
        return
    fi
    exit_pid=$(pidof $SERVER_NAME)
    if [[ $exit_pid != "" ]]; then
        echo "Now kill the server by pid:" $exit_pid
        kill -9 $exit_pid
    fi
}

function build_doxygen_docs() {
    # build documentation only for cpp
    cd $CLIENT_OUTPUT_DIR/client
    rm -rf doc/html doc/latex
    doxygen doc/Doxyfile.in
    cd $OPENAPI_HOME
}

testFolders=("petstore" "colorpalette" "operation-parameters" "openapi2.0" "mediatype")
for i in "${testFolders[@]}"
do
    CLIENTFOLDER_NAME=client
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$i/$CLIENTFOLDER_NAME"
    run_test_server $i ;
    run_test ;
    kill_test_server ;
    build_doxygen_docs ;
done

qmlTestFolders=("petstore" "colorpalette")
for i in "${qmlTestFolders[@]}"
do
    CLIENTFOLDER_NAME=qmlclient
    CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$i/$CLIENTFOLDER_NAME"
    run_test_server $i ;
    run_test ;
    kill_test_server ;
done

