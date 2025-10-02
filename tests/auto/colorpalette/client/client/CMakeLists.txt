cmake_minimum_required(VERSION 3.16)

project(ColorpaletteClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

qt_add_library(${PROJECT_NAME}
    QtOAIColor.h
    QtOAIColorPage.h
    QtOAITestObject.h
    QtOAITestOperation_request.h
    QtOAIUpdateUser_request.h
    QtOAIUser.h
    QtOAIUserPage.h
    QtOAIColorsApi.h
    QtOAIDefaultApi.h
    QtOAIRegisterApi.h
    QtOAIUsersApi.h
    QtOAIBaseApi.h
    QtOAIHelpers.h
    QtOAIHttpRequest.h
    QtOAIObject.h
    QtOAIEnum.h
    QtOAIHttpFileElement.h
    QtOAIServerConfiguration.h
    QtOAIServerVariable.h
    QtOAIColor.cpp
    QtOAIColorPage.cpp
    QtOAITestObject.cpp
    QtOAITestOperation_request.cpp
    QtOAIUpdateUser_request.cpp
    QtOAIUser.cpp
    QtOAIUserPage.cpp
    QtOAIColorsApi.cpp
    QtOAIDefaultApi.cpp
    QtOAIRegisterApi.cpp
    QtOAIUsersApi.cpp
    QtOAIBaseApi.cpp
    QtOAIHelpers.cpp
    QtOAIHttpRequest.cpp
    QtOAIHttpFileElement.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Network
        Qt::Gui
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
    FILES ${HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
    EXPORT ${PROJECT_NAME}Targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
