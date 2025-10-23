QT += network

HEADERS += \
# Models
    $${PWD}/OAIColor.h \
    $${PWD}/OAIColorPage.h \
    $${PWD}/OAITestObject.h \
    $${PWD}/OAITestOperation_request.h \
    $${PWD}/OAIUpdateUser_request.h \
    $${PWD}/OAIUser.h \
    $${PWD}/OAIUserPage.h \
# APIs
    $${PWD}/OAIColorsApi.h \
    $${PWD}/OAIDefaultApi.h \
    $${PWD}/OAIRegisterApi.h \
    $${PWD}/OAIUsersApi.h \

SOURCES += \
# Models
    $${PWD}/OAIColor.cpp \
    $${PWD}/OAIColorPage.cpp \
    $${PWD}/OAITestObject.cpp \
    $${PWD}/OAITestOperation_request.cpp \
    $${PWD}/OAIUpdateUser_request.cpp \
    $${PWD}/OAIUser.cpp \
    $${PWD}/OAIUserPage.cpp \
# APIs
    $${PWD}/OAIColorsApi.cpp \
    $${PWD}/OAIDefaultApi.cpp \
    $${PWD}/OAIRegisterApi.cpp \
    $${PWD}/OAIUsersApi.cpp \
