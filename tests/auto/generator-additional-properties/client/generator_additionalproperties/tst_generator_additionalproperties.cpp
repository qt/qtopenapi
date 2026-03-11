// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"
#include "qoaihttprequest.h"

#include "addproptestapi.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtemporaryfile.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

class tst_GeneratorAdditionalProperties : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void addDownloadProgress();
    void ensureUniqueParamsTest();
    void cleanupTestCase();

private:
    QProcess m_serverProcess;
};

void tst_GeneratorAdditionalProperties::initTestCase()
{
    if (m_serverProcess.state() != QProcess::ProcessState::Running) {
        m_serverProcess.start(SERVER_PATH);
        if (!m_serverProcess.waitForStarted()) {
            qFatal() << "Couldn't start the server: " << m_serverProcess.errorString();
            exit(EXIT_FAILURE);
        }
        // give the process some time to properly start up the server
        QThread::currentThread()->msleep(1000);
    }
}

void tst_GeneratorAdditionalProperties::addDownloadProgress()
{
    bool done = false;
    bool progressReceived = false;
    qint64 lastTotal = -1;

    QTemporaryFile tmp;
    QVERIFY2(tmp.open(), "Failed to create temporary file.");
    tmp.write(QByteArray(500 * 1024, 'x'));
    tmp.flush();

    const QString filePath = tmp.fileName();
    QVERIFY(!filePath.isEmpty());
    const QtOpenApiCommon::QOAIHttpFileElement file(filePath);

    AddPropNamespace::AddPropTestApi api;

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripFinished,
            this, [&](const QtOpenApiCommon::QOAIHttpFileElement &) {
                done = true;
            });

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripErrorOccurred,
            this, [&](QNetworkReply::NetworkError errType, const QString &errStr) {
                done = false;
                qCritical() << errType << errStr;
            });

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripProgress,
            this, [&](qint64 bytesReceived, qint64 bytesTotal) {
                progressReceived = true;
                lastTotal = bytesTotal;

                //qDebug() << "bytesReceived =" << bytesReceived << "bytesTotal =" << bytesTotal;
                QVERIFY(bytesReceived > 0);
                QVERIFY(bytesTotal > 0 || bytesTotal == -1); // -1 means unknown size
            });

    api.roundtrip(file);

    QTRY_COMPARE_EQ(done, true);
    QVERIFY(progressReceived);
    QVERIFY(lastTotal > 0);
}

void tst_GeneratorAdditionalProperties::ensureUniqueParamsTest()
{
    // Although the 'testUniqueParams' operation defines two parameters with the same name in the
    // spec, the generator creates unique parameter names because 'ensureUniqueParams' is true.
    AddPropNamespace::AddPropTestApi api;
    QTest::ignoreMessage(QtWarningMsg, "Access manager destroyed while 1 requests were still in "
                                       "progress");
    api.testUniqueParams(2, "id2"_L1);
}

void tst_GeneratorAdditionalProperties::cleanupTestCase()
{
    if (m_serverProcess.state() == QProcess::ProcessState::Running) {
        m_serverProcess.kill();
        m_serverProcess.waitForFinished();
    }
}

QTEST_MAIN(tst_GeneratorAdditionalProperties)
#include "tst_generator_additionalproperties.moc"
