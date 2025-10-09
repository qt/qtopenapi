// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/QtOAITestApi.h"

#include <QtCore/qobject.h>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

#define CALL_TEST_PDF_OPERATION(OPERATION, PARAM, EXPECTED_CONTENT, EXPECTED_FILENAME)          \
{                                                                                               \
    bool done = false;                                                                          \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QtOAIHttpFileElement &summary) {  \
        if (!(done = reply.isSuccess())) {                                                      \
            qWarning() << "Error happened while issuing request:" << reply.error()              \
                       << reply.errorString();                                                  \
        }                                                                                       \
        QVERIFY(summary.isSet());                                                               \
        if (!EXPECTED_FILENAME.startsWith("unnamed"_L1))                                        \
            QCOMPARE(summary.requestFilename(), EXPECTED_FILENAME);                             \
        QCOMPARE(summary.loadFromLocalFile(), EXPECTED_CONTENT);                                \
    });                                                                                         \
    QTRY_COMPARE_EQ(done, true);                                                                \
}

namespace QtOpenAPI {

static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.setWorkingDirectory(SERVER_DIR);
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

static QJsonValue getObjectValue(const QString &summary, const QString &key)
{
    const QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        const QJsonObject obj = doc.object();
        return obj.value(key);
    }
    return QJsonValue();
}

class Responses : public QtOAITestApi {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void jsonResponse();
    void textResponse();
    void pdfResponse();
    void cleanupTestCase();
};

void Responses::jsonResponse() {
    bool done = false;

    // JSON response
    applicationJsonStringResponse(this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess())) {
                qWarning() << "Error happened while issuing request : " << reply.error()
                           << reply.errorString();
        }
        QCOMPARE(getObjectValue(summary, "status"_L1).toString(), "OK"_L1);
    });
    QTRY_COMPARE_EQ(done, true);

    done = false;

    applicationJsonObjectResponse(
        this, [&](const QRestReply &reply,
                  const QtOAIApplicationJsonObjectResponse_200_response &summary) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "Error happened while issuing request : " << reply.error()
                       << reply.errorString();
        }
        QCOMPARE(summary.getStatus(), "OK"_L1);
        QCOMPARE(summary.getValue(), 22);
    });
    QTRY_COMPARE_EQ(done, true);
}

void Responses::textResponse() {
    bool done = false;

    // Plain text response
    textPlainStringResponse(this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "Error happened while issuing request : " << reply.error()
                       << reply.errorString();
        }
        QCOMPARE(summary, "Hello plain text"_L1);
    });
    QTRY_COMPARE_EQ(done, true);
}

void Responses::pdfResponse() {
    QString filePath = QDir(SERVER_DIR).filePath("test.pdf"_L1);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        QFAIL("Failed to open expected PDF file");
    QByteArray expectedPdfContent = file.readAll();
    file.close();

    CALL_TEST_PDF_OPERATION(applicationPdfInlineResponse, "test.pdf"_L1, expectedPdfContent,
                            "unnamed"_L1);
    CALL_TEST_PDF_OPERATION(applicationPdfSaveResponse, "test.pdf"_L1, expectedPdfContent,
                            "example1.pdf"_L1);

    // TODO: Change expected content to "expectedPdfContent" when we activate compression
    //       in a later step. For now compression flag is off.
    QTest::ignoreMessage(QtWarningMsg, "Content compression is disabled: contentCompression flag "
                                       "is off. Returning an empty QByteArray.");
    CALL_TEST_PDF_OPERATION(applicationEncodedPdfSaveResponse, "test.pdf"_L1, "",
                            "compressed_example1.pdf"_L1);
}

void Responses::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // namespace QtOpenAPI

QTEST_MAIN(QtOpenAPI::Responses)
#include "Responses.moc"
