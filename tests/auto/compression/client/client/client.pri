QT += network

HEADERS += \
# Models
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
# APIs
    $${PWD}/QtOAITestApi.cpp \
# Others
    $${PWD}/QtOAIHelpers.cpp \
    $${PWD}/QtOAIHttpRequest.cpp \
    $${PWD}/QtOAIHttpFileElement.cpp
