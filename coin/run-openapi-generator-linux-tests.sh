#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# petstore test server uses the old Qt5 libs for the server implementation
# TBD QTBUG-137879: remove dependency to the old Qt5 libs, and refactore current server or implement a new one
sudo apt-get -y install qtbase5-dev qtbase5-dev-tools
# Qt6 generator uses doxygen to provide documentation to the user
sudo apt-get -y install doxygen graphviz
OPENAPI_HOME=/home/qt/work/playground/qtopenapi
export CMAKE_PREFIX_PATH="/home/qt/work/install"

function run_test() {
    #build and run client test apps
    cd $CLIENT_OUTPUT_DIR/
    rm -rf $CLIENT_OUTPUT_DIR/build
    source build-and-test.bash
}

function killPetServer() {
    # when the client finished testing, let's kill server ]:->
    exit_pid=$(pidof cpp-qt-qhttpengine-server)
    echo "Now kill the server by pid:" $exit_pid
    kill -9 $exit_pid
}

function build_doxygen_docs() {
    # build documentation only for cpp
    cd $CLIENT_OUTPUT_DIR/client
    rm -rf doc/html doc/latex
    doxygen doc/Doxyfile.in
    cd $OPENAPI_HOME
}
# build and run server app
SERVER_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/petstore/server"
cd $SERVER_OUTPUT_DIR
rm -rf $SERVER_OUTPUT_DIR/build
source build-and-run.bash
# build and run petsore cpp client
CLIENTFOLDER_NAME=client
CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/petstore/$CLIENTFOLDER_NAME"
run_test ;
build_doxygen_docs ;

# build and run petsore qml client
CLIENTFOLDER_NAME=qmlclient
CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/petstore/$CLIENTFOLDER_NAME"
run_test ;
killPetServer ;

# generate colorpalette cpp client
CLIENTFOLDER_NAME=client
CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/colorpalette/$CLIENTFOLDER_NAME"
rm -rf CLIENT_OUTPUT_DIR/client
run_test ;
build_doxygen_docs ;

# generate colorpalette qml client
CLIENTFOLDER_NAME=qmlclient
CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/colorpalette/$CLIENTFOLDER_NAME"
rm -rf CLIENT_OUTPUT_DIR/client
run_test ;

