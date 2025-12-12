// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/testapi.h"

#include <QtCore/qobject.h>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtCommonOpenAPI;

#define CALL_TEST_FILE_OPERATION(OPERATION, PARAM, EXPECTED_CONTENT, EXPECTED_FILENAME)         \
{                                                                                               \
    bool done = false;                                                                          \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QOAIHttpFileElement &summary) {  \
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

static QByteArray readFile(const QString &filename)
{
    QByteArray fileContent;
    QString filePath = QDir(SERVER_DIR).filePath(filename);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        qWarning("Failed to open %s", qPrintable(filename));
    else
        fileContent = file.readAll();

    file.close();
    return fileContent;
}

class Responses : public TestApi {
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
    void imageResponse();
    void octetStreamResponse();
    void emptyResponseBody();
    void cleanupTestCase();
};

void Responses::jsonResponse() {
    bool done = false;

    applicationJsonStringResponse(this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess())) {
                qWarning() << "Error happened while issuing request : " << reply.error()
                           << reply.errorString();
        }
        QCOMPARE(getObjectValue(summary, "status"_L1).toString(), "OK"_L1);
    });
    QTRY_COMPARE_EQ(done, true);

    User user1, user2;
    user1.setName("user1");
    user1.setId(1);
    user2.setName("user2");
    user2.setId(2);

    done = false;
    applicationJsonArrayResponse(this, [&](const QRestReply &reply,
                                           const QList<User> &summary) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "Error happened while issuing request : " << reply.error()
                       << reply.errorString();
        }
        QCOMPARE(summary[0].getName(), user1.getName());
        QCOMPARE(summary[0].getId(), user1.getId());
        QCOMPARE(summary[1].getName(), user2.getName());
        QCOMPARE(summary[1].getId(), user2.getId());
    });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    applicationJsonMapResponse(this, [&](const QRestReply &reply,
                                           const QMap<QString, User> &summary) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "Error happened while issuing request : " << reply.error()
                       << reply.errorString();
        }
        QVERIFY(summary.size() == 2);
        QCOMPARE(summary["first"].getName(), user1.getName());
        QCOMPARE(summary["first"].getId(), user1.getId());
        QCOMPARE(summary["second"].getName(), user2.getName());
        QCOMPARE(summary["second"].getId(), user2.getId());
    });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    applicationJsonEncodedObjectResponse(
        this, [&](const QRestReply &reply,
                  const ApplicationJsonEncodedObjectResponse_200_response &summary) {
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
    QByteArray expectedPdfContent = readFile("test.pdf"_L1);

    CALL_TEST_FILE_OPERATION(applicationPdfInlineResponse, "test.pdf"_L1, expectedPdfContent,
                            "unnamed"_L1);
    CALL_TEST_FILE_OPERATION(applicationPdfSaveResponse, "test.pdf"_L1, expectedPdfContent,
                            "example1.pdf"_L1);

    CALL_TEST_FILE_OPERATION(applicationEncodedPdfSaveResponse, "test.pdf"_L1, expectedPdfContent,
                            "compressed_example1.pdf"_L1);
}

void Responses::imageResponse() {
    QByteArray expectedImage = readFile("testImage.jpg"_L1);
    CALL_TEST_FILE_OPERATION(inlineImageResponse, "jpegImage"_L1, expectedImage, "unnamed"_L1);
    CALL_TEST_FILE_OPERATION(saveImageResponse, "jpegImage"_L1, expectedImage, "example2.jpg"_L1);

    expectedImage = readFile("testImage.png"_L1);
    CALL_TEST_FILE_OPERATION(inlineImageResponse, "pngImage"_L1, expectedImage, "unnamed"_L1);
    CALL_TEST_FILE_OPERATION(saveImageResponse, "pngImage"_L1, expectedImage, "example3.png"_L1);
}

void Responses::octetStreamResponse() {
    QByteArray expectedBinData = readFile("test.bin"_L1);;
    CALL_TEST_FILE_OPERATION(applicationOctetStreamResponse, "test.bin"_L1, expectedBinData,
                             "example.bin"_L1);
}

void Responses::emptyResponseBody() {
    bool done = false;
    emptyResponse(this, [&](const QRestReply &reply) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "Error happened while issuing request : " << reply.error()
                       << reply.errorString();
        }
        QCOMPARE(reply.httpStatus(), 204);
    });
    QTRY_COMPARE_EQ(done, true);
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
