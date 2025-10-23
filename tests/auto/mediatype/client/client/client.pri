QT += network

HEADERS += \
# Models
    $${PWD}/QtOAIPostApplicationJsonSeveralObjects_request.h \
    $${PWD}/QtOAIPostMultiPartData_request_formObject.h \
    $${PWD}/QtOAIUser.h \
# APIs
    $${PWD}/QtOAITestApi.h \

SOURCES += \
# Models
    $${PWD}/QtOAIPostApplicationJsonSeveralObjects_request.cpp \
    $${PWD}/QtOAIPostMultiPartData_request_formObject.cpp \
    $${PWD}/QtOAIUser.cpp \
# APIs
    $${PWD}/QtOAITestApi.cpp \
