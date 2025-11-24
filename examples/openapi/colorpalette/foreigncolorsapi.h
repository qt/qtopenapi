// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FOREIGNCOLORSAPI_H
#define FOREIGNCOLORSAPI_H

#include <QtCore/qobject.h>
#include <QtQml/qjsengine.h>
#include <QtQml/qqml.h>

#include "QtOAIColorsApi.h"

struct ForeignColorsAPI
{
    Q_GADGET
    QML_FOREIGN(QtOpenAPI::QtOAIColorsApi)
    QML_SINGLETON
    QML_NAMED_ELEMENT(ColorsApi)

public:
    inline static QtOpenAPI::QtOAIColorsApi *s_singletonInstance = nullptr;

    static QtOpenAPI::QtOAIColorsApi *create(QQmlEngine *, QJSEngine *engine)
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

#endif // FOREIGNCOLORSAPI_H
