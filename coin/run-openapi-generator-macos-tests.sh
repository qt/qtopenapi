#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# petstore test server uses the old Qt5 libs for the server implementation
# TBD QTBUG-137879: remove dependency to the old Qt5 libs, and refactore current server or implement a new one
brew install qt@5

OPENAPI_HOME=/Users/qt/work/playground/qtopenapi
export CMAKE_PREFIX_PATH="/Users/qt/work/install"
ARCH_TYPE=$(machine)
echo "ARCH_TYPE=" $ARCH_TYPE
if [[ $ARCH_TYPE == *"86_64"* ]]; then
    export LDFLAGS="-L/usr/local/opt/qt@5/lib -L/usr/local/openssl-3.0.7/lib"
    export CPPFLAGS="-I/usr/local/opt/qt@5/include"
    export PATH="/usr/local/opt/qt@5/bin:$PATH"
else
    export LDFLAGS="-L/opt/homebrew/opt/qt@5/lib -L/usr/local/openssl-3.0.7/lib"
    export CPPFLAGS="-I/opt/homebrew/opt/qt@5/include"
    export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
fi

function run_test() {
    #build and run client test apps
    cd $CLIENT_OUTPUT_DIR/
    rm -rf $CLIENT_OUTPUT_DIR/build
    source build-and-test.bash
}

function killPetServer() {
    # when the client finished testing, let's kill server ]:->
    exit_pid=$(pgrep cpp-qt-qhttpengine-server)
    echo "Now kill the server by pid:" $exit_pid
    kill -9 $exit_pid
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

# generate colorpalette qml client
CLIENTFOLDER_NAME=qmlclient
CLIENT_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/colorpalette/$CLIENTFOLDER_NAME"
rm -rf CLIENT_OUTPUT_DIR/client
run_test ;

