#!/bin/bash

set -e

export CMAKE_POLICY_VERSION_MINIMUM=3.5
#export CMAKE_PREFIX_PATH="" Path to Qt5 on your desktop

#sudo apt install libssl-dev qtbase5-dev qtbase5-dev-tools curl

if ! command -V curl 2>&1 >/dev/null
then
    echo "'Curl' is not installed. Run 'sudo apt install curl' for installation on Linux."
    exit 1
fi

mkdir -p build
cd build

cmake .. -G Ninja

cmake --build . --parallel

$PWD/src/cpp-qt-qhttpengine-server -p 9080 -a 127.0.0.1 &
