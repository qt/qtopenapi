// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/QtOAITestApi.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

namespace QtOpenAPI {

static QJsonValue getJsonValue(const QString &summary, const QString &key = "status"_L1)
{
    const QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        const QJsonObject obj = doc.object();
        return obj.value(key);
    }
    return QJsonValue();
}

class tst_ServerConfiguration : public QtOAITestApi {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void substituteEnumVariable_data();
    void substituteEnumVariable();
    void substituteNormalVariable_data();
    void substituteNormalVariable();

private:
    QProcess m_process;
};

void tst_ServerConfiguration::initTestCase()
{
    if (m_process.state() != QProcess::ProcessState::Running) {
        m_process.start(SERVER_PATH);
        if (!m_process.waitForStarted()) {
            qFatal() << "Couldn't start the server: " << m_process.errorString();
            exit(EXIT_FAILURE);
        }
        // give the process some time to properly start up the server
        QThread::currentThread()->msleep(1000);
    }
}

void tst_ServerConfiguration::cleanupTestCase()
{
    if (m_process.state() == QProcess::ProcessState::Running) {
        m_process.kill();
        m_process.waitForFinished();
    }
}

void tst_ServerConfiguration::substituteEnumVariable_data()
{
    QTest::addColumn<QString>("basePath");
    QTest::addColumn<bool>("expectSuccess");

    QTest::addRow("v1-correct") << u"v1"_s << true;
    QTest::addRow("v2-correct") << u"v2"_s << true;
    QTest::addRow("v3-wrong") << u"v3"_s << false;
}

void tst_ServerConfiguration::substituteEnumVariable()
{
    QFETCH(const QString, basePath);
    QFETCH(const bool, expectSuccess);

    // If the value is not in the enum, the variable simply won't be changed.
    const auto error = setDefaultServerValue(0, u"dummyOperation"_s, u"basePath"_s, basePath);
    if (expectSuccess)
        QCOMPARE_EQ(error, QtOAITestApi::ServerError::NoError);
    else
        QCOMPARE_EQ(error, QtOAITestApi::ServerError::EnumValueNotFound);

    if (expectSuccess) {
        bool done = false;
        dummyOperation(this, [&](const QRestReply &reply, const QString &summary) {
            if (!(done = reply.isSuccess()))
                qWarning() << "ERROR: " << reply.errorString() << reply.error();
            const QString expectedString = u"/%1/operations/dummy"_s.arg(basePath);
            QCOMPARE(getJsonValue(summary).toString(), expectedString);
        });
        QTRY_COMPARE_EQ(done, true);
    }
}

void tst_ServerConfiguration::substituteNormalVariable_data()
{
    QTest::addColumn<QString>("port");
    QTest::addColumn<bool>("expectSuccess");

    QTest::addRow("20303_correct") << u"20303"_s << true;
    QTest::addRow("20304_wrong") << u"20304"_s << false;
}

void tst_ServerConfiguration::substituteNormalVariable()
{
    QFETCH(const QString, port);
    QFETCH(const bool, expectSuccess);

    // First, set basePath to "v1" to have a consistent result
    QCOMPARE_EQ(setDefaultServerValue(0, u"dummyOperation"_s, u"basePath"_s, u"v1"_s),
                QtOAITestApi::ServerError::NoError);

    // Now set the port. Since the variable does not have an enum in its
    // definition, any value should be accepted.
    QCOMPARE_EQ(setDefaultServerValue(0, u"dummyOperation"_s, u"port"_s, port),
                QtOAITestApi::ServerError::NoError);

    // Then if the port is correct, the operation executes successfully.
    // Otherwise we fail to connect to the server.
    if (expectSuccess) {
        bool done = false;
        dummyOperation(this, [&](const QRestReply &reply, const QString &summary) {
            if (!(done = reply.isSuccess()))
                qWarning() << "ERROR: " << reply.errorString() << reply.error();
            QCOMPARE(getJsonValue(summary).toString(), u"/v1/operations/dummy"_s);
        });
        QTRY_COMPARE_EQ(done, true);
    } else {
        QNetworkReply::NetworkError error = QNetworkReply::NoError;
        dummyOperation(this, [&](const QRestReply &reply, const QString &) {
            error = reply.error();
        });
        QTRY_COMPARE_NE(error, QNetworkReply::NoError);
    }
}

} // namespace QtOpenAPI

QTEST_MAIN(QtOpenAPI::tst_ServerConfiguration)
#include "tst_server_configuration.moc"
