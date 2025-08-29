QT += network

HEADERS += \
# Models
    $${PWD}/QtOAIColor.h \
    $${PWD}/QtOAIColorPage.h \
    $${PWD}/QtOAITestObject.h \
    $${PWD}/QtOAITestOperation_request.h \
    $${PWD}/QtOAIUpdateUser_request.h \
    $${PWD}/QtOAIUser.h \
    $${PWD}/QtOAIUserPage.h \
# APIs
    $${PWD}/QtOAIColorsApi.h \
    $${PWD}/QtOAIDefaultApi.h \
    $${PWD}/QtOAIRegisterApi.h \
    $${PWD}/QtOAIUsersApi.h \
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
    $${PWD}/QtOAIColor.cpp \
    $${PWD}/QtOAIColorPage.cpp \
    $${PWD}/QtOAITestObject.cpp \
    $${PWD}/QtOAITestOperation_request.cpp \
    $${PWD}/QtOAIUpdateUser_request.cpp \
    $${PWD}/QtOAIUser.cpp \
    $${PWD}/QtOAIUserPage.cpp \
# APIs
    $${PWD}/QtOAIColorsApi.cpp \
    $${PWD}/QtOAIDefaultApi.cpp \
    $${PWD}/QtOAIRegisterApi.cpp \
    $${PWD}/QtOAIUsersApi.cpp \
# Others
    $${PWD}/QtOAIHelpers.cpp \
    $${PWD}/QtOAIHttpRequest.cpp \
    $${PWD}/QtOAIHttpFileElement.cpp
