QT += network

HEADERS += \
# Models
    $${PWD}/OAIApiResponse.h \
    $${PWD}/OAICategory.h \
    $${PWD}/OAIOrder.h \
    $${PWD}/OAIPet.h \
    $${PWD}/OAITag.h \
    $${PWD}/OAIUser.h \
# APIs
    $${PWD}/OAIPetApi.h \
    $${PWD}/OAIStoreApi.h \
    $${PWD}/OAIUserApi.h \

SOURCES += \
# Models
    $${PWD}/OAIApiResponse.cpp \
    $${PWD}/OAICategory.cpp \
    $${PWD}/OAIOrder.cpp \
    $${PWD}/OAIPet.cpp \
    $${PWD}/OAITag.cpp \
    $${PWD}/OAIUser.cpp \
# APIs
    $${PWD}/OAIPetApi.cpp \
    $${PWD}/OAIStoreApi.cpp \
    $${PWD}/OAIUserApi.cpp \
