// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef APIBRIDGES_H
#define APIBRIDGES_H

#include <QtCore/qobject.h>
#include <QtQml/qjsengine.h>
#include <QtQml/qqml.h>

#include "Color.h"
#include "ColorsApi.h"
#include "Credentials.h"
#include "UsersApi.h"

struct ForeignColorsAPI
{
    Q_GADGET
    QML_FOREIGN(QtOpenAPI::ColorsApi)
    QML_SINGLETON
    QML_NAMED_ELEMENT(ColorsApi)

public:
    inline static QtOpenAPI::ColorsApi *s_singletonInstance = nullptr;

    static QtOpenAPI::ColorsApi *create(QQmlEngine *, QJSEngine *engine)
    {
        // The instance has to exist before it is used. We cannot replace it.
        Q_ASSERT(s_singletonInstance);

        // The engine has to have the same thread affinity as the singleton.
        Q_ASSERT(engine->thread() == s_singletonInstance->thread());

        // There can only be one engine accessing the singleton.
        if (s_engine)
            Q_ASSERT(engine == s_engine);
        else
            s_engine = engine;

        // Explicitly specify C++ ownership so that the engine doesn't delete
        // the instance.
        QJSEngine::setObjectOwnership(s_singletonInstance,
                                      QJSEngine::CppOwnership);
        return s_singletonInstance;
    }

private:
    inline static QJSEngine *s_engine = nullptr;
};

struct ForeignUsersAPI
{
    Q_GADGET
    QML_FOREIGN(QtOpenAPI::UsersApi)
    QML_SINGLETON
    QML_NAMED_ELEMENT(UsersApi)

public:
    inline static QtOpenAPI::UsersApi *s_singletonInstance = nullptr;

    static QtOpenAPI::UsersApi *create(QQmlEngine *, QJSEngine *engine)
    {
        // The instance has to exist before it is used. We cannot replace it.
        Q_ASSERT(s_singletonInstance);

        // The engine has to have the same thread affinity as the singleton.
        Q_ASSERT(engine->thread() == s_singletonInstance->thread());

        // There can only be one engine accessing the singleton.
        if (s_engine)
            Q_ASSERT(engine == s_engine);
        else
            s_engine = engine;

        // Explicitly specify C++ ownership so that the engine doesn't delete
        // the instance.
        QJSEngine::setObjectOwnership(s_singletonInstance,
                                      QJSEngine::CppOwnership);
        return s_singletonInstance;
    }

private:
    inline static QJSEngine *s_engine = nullptr;
};

class Credentials: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    Q_INVOKABLE QtOpenAPI::Credentials create(const QString &email, const QString &password)
    {
        QtOpenAPI::Credentials credentials;
        credentials.setEmail(email);
        credentials.setPassword(password);
        return credentials;
    }
};

class Color: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    Q_INVOKABLE QtOpenAPI::Color create(const QJsonObject &json)
    {
        QtOpenAPI::Color color;
        color.fromJsonObject(json);
        return color;
    }
};

#endif // APIBRIDGES_H
