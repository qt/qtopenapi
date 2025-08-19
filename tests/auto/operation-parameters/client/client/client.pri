QT += network

HEADERS += \
# Models
    $${PWD}/QtOAIDoubleResponse.h \
    $${PWD}/QtOAIFloatResponse.h \
    $${PWD}/QtOAITestObject.h \
# APIs
    $${PWD}/QtOAITestApi.h \
# Others
    $${PWD}/QtOAIHelpers.h \
    $${PWD}/QtOAIHttpRequest.h \
    $${PWD}/QtOAIObject.h \
    $${PWD}/QtOAIEnum.h \
    $${PWD}/QtOAIHttpFileElement.h \
    $${PWD}/QtOAIServerConfiguration.h \
    $${PWD}/QtOAIServerVariable.h

SOURCES += \
# Models
    $${PWD}/QtOAIDoubleResponse.cpp \
    $${PWD}/QtOAIFloatResponse.cpp \
    $${PWD}/QtOAITestObject.cpp \
# APIs
    $${PWD}/QtOAITestApi.cpp \
# Others
    $${PWD}/QtOAIHelpers.cpp \
    $${PWD}/QtOAIHttpRequest.cpp \
    $${PWD}/QtOAIHttpFileElement.cpp
