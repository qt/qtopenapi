QT += network

HEADERS += \
# Models
    $${PWD}/QtOAIApiResponse.h \
    $${PWD}/QtOAICategory.h \
    $${PWD}/QtOAIOrder.h \
    $${PWD}/QtOAIPet.h \
    $${PWD}/QtOAITag.h \
    $${PWD}/QtOAIUser.h \
# APIs
    $${PWD}/QtOAIPetApi.h \
    $${PWD}/QtOAIStoreApi.h \
    $${PWD}/QtOAIUserApi.h \

SOURCES += \
# Models
    $${PWD}/QtOAIApiResponse.cpp \
    $${PWD}/QtOAICategory.cpp \
    $${PWD}/QtOAIOrder.cpp \
    $${PWD}/QtOAIPet.cpp \
    $${PWD}/QtOAITag.cpp \
    $${PWD}/QtOAIUser.cpp \
# APIs
    $${PWD}/QtOAIPetApi.cpp \
    $${PWD}/QtOAIStoreApi.cpp \
    $${PWD}/QtOAIUserApi.cpp \
