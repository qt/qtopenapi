QT += network

HEADERS += \
# Models
    $${PWD}/QtOAIPostApplicationJsonSeveralObjects_request.h \
    $${PWD}/QtOAIPostMultiPartData_request_formObject.h \
    $${PWD}/QtOAIUser.h \
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
    $${PWD}/QtOAIPostApplicationJsonSeveralObjects_request.cpp \
    $${PWD}/QtOAIPostMultiPartData_request_formObject.cpp \
    $${PWD}/QtOAIUser.cpp \
# APIs
    $${PWD}/QtOAITestApi.cpp \
# Others
    $${PWD}/QtOAIHelpers.cpp \
    $${PWD}/QtOAIHttpRequest.cpp \
    $${PWD}/QtOAIHttpFileElement.cpp
