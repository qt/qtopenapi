#!/bin/bash
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
set -e

# Qt6 generator uses doxygen to provide documentation to the user
brew install graphviz doxygen

# operation-parameters test server uses Go
brew install golang
# Fetch missing Go dependencies (only works if run from within a Go module)
if [[ -f "go.mod" ]]; then
    go mod tidy
fi

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

function run_test_server() {
    SERVER_OUTPUT_DIR="$OPENAPI_HOME/tests/auto/$1/server"
    if [[ $1 == "petstore" ]]; then
        SERVER_NAME="petstore-server-app"
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
    exit_pid=$(pgrep $SERVER_NAME)
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

