QT += network

HEADERS += \
# Models
    $${PWD}/OAIPostApplicationJsonSeveralObjects_request.h \
    $${PWD}/OAIPostMultiPartData_request_formObject.h \
    $${PWD}/OAIUser.h \
# APIs
    $${PWD}/OAITestApi.h \
# Others
    $${PWD}/OAIHelpers.h \
    $${PWD}/OAIHttpRequest.h \
    $${PWD}/OAIObject.h \
    $${PWD}/OAIEnum.h \
    $${PWD}/OAIHttpFileElement.h \
    $${PWD}/OAIServerConfiguration.h \
    $${PWD}/OAIServerVariable.h

SOURCES += \
# Models
    $${PWD}/OAIPostApplicationJsonSeveralObjects_request.cpp \
    $${PWD}/OAIPostMultiPartData_request_formObject.cpp \
    $${PWD}/OAIUser.cpp \
# APIs
    $${PWD}/OAITestApi.cpp \
# Others
    $${PWD}/OAIHelpers.cpp \
    $${PWD}/OAIHttpRequest.cpp \
    $${PWD}/OAIHttpFileElement.cpp
