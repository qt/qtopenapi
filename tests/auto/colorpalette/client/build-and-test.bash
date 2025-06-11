#!/bin/bash
set -e # Exit immediately if a command fails

mkdir -p build
cd build
# Run CMake to configure the project
cmake .. -G Ninja
# Compile the project
cmake --build . --parallel
apps=("colorpalettetest/cpp-qt-colorpalette-app")
folders=("colorpalettetest")
if [[ -z "${RUN_VALGRIND_TESTS}" ]]; then
    for i in "${folders[@]}"
    do
        echo "Running Qt ColorPalette Test: " $i
        cd $i
        ctest
        cd ..
    done
else
    echo "Running Qt Petstore Tests with Valgrind"
    for i in "${apps[@]}"
    do
        echo $PWD/$i
        valgrind --leak-check=full $PWD/$i 2>&1 tee result.log || exit 1
        testCount=$(cat result.log | grep 'Finished testing of' | wc -l)
        if [ $testCount == 3 ]
        then
            echo "Ok"
            else
            echo "The tests were not run!!!"
            exit 1
        fi

        echo "Make sure the tests passed:"
        successCount=$(cat result.log | grep '0 failed' | wc -l)
        if [ $successCount == 3 ]
        then
            echo "Ok"
            else
            echo "The tests failed!!!"
            exit 1
        fi

        echo "Check if no memory leaks occurred:"
        leakCount=$(cat result.log | grep 'lost: 0 bytes in 0 blocks' | wc -l)
        if [ $leakCount == 3 ]
        then
            echo "Ok"
            else
            echo "There was memory leaks!!!"
            exit 1
        fi
    done
fi
