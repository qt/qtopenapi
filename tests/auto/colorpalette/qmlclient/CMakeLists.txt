cmake_minimum_required(VERSION 3.16)

project(qml-qt-colorpalette)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS QuickTest)
add_subdirectory(client)

qt_standard_project_setup(REQUIRES 6.8)
qt_add_executable(qml-qt-colorpalette
    colorpalettetest/main.cpp
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(qml-qt-colorpalette
    URI main.uri
    VERSION 1.0
    QML_FILES
        colorpalettetest/tst_colorpalette.qml
)

target_include_directories(qml-qt-colorpalette
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/client>
)

target_link_libraries(qml-qt-colorpalette
    PUBLIC
        Qt6OpenAPIClient
        Qt::QuickTest
)

enable_testing()
add_test(NAME qml-qt-colorpalette-test COMMAND qml-qt-colorpalette)
