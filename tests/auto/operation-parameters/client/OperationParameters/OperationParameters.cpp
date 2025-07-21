// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/OAITestApi.h"

#include <QtCore/qobject.h>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

namespace OpenAPI {

class OperationParameters : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void pathParams();
};

void OperationParameters::pathParams() {
    OAITestApi api;
    bool done = false;

    // style=form, explode=true
    connect(&api, &OAITestApi::simpleExplodeStringFinished, [&](const QString &jsonPath){
        done = true;
        QJsonDocument doc = QJsonDocument::fromJson(jsonPath.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
             QJsonObject obj = doc.object();
             QString path = obj.value("status").toString();
             QCOMPARE(path, "/v2/path/primitive/simple-explode/hello");
         }
    });
    api.simpleExplodeString("hello");
    QTRY_COMPARE_EQ_WITH_TIMEOUT(done, true, 5000);
}

} // OpenAPI

QTEST_MAIN(OpenAPI::OperationParameters)
#include "OperationParameters.moc"
