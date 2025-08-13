// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/OAITestApi.h"

#include <QtCore/qobject.h>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

namespace OpenAPI {

QJsonValue getJsonValue(const QString &summary, const QString &key = "status")
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        return obj.value(key);
    }
    return QJsonValue();
}

QString getHeaderValue(const QString &summary)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        QStringList headers = obj.value("header").toVariant().toMap().value("Content-Type").toStringList();
        if (headers.size() > 0)
            return headers.at(0);
    }
    return QString();
}

OAIUser getUserByStatusObject(const QString &summary)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    OAIUser user;
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.value("json-object").isObject())
            user.fromJsonObject(obj.value("json-object").toObject());
    }
    return user;
}

class MediaType : public OAITestApi {
    Q_OBJECT

private Q_SLOTS:
    void testJsonMediaType();
    void testPlainText_data();
    void testPlainText();
    void testOctetStream();
};

// MEDIA TYPE `application/json`
void MediaType::testJsonMediaType()
{
    bool done = false;
    // See JSON supported types https://spec.openapis.org/oas/v3.1.1.html#data-types
    // NOTE: encoding is NOT being applied for `application/json` media type by SPEC.
    // NOTE: Primitive numbers or bool in JSONs look like a simple value: 42 or false, or "TEXT"
    const QString appJsonHeader("application/json");
    postApplicationJsonBool(true, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "bool").toBool(), true);
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, "true");
    QTRY_COMPARE_EQ(done, true);

    done = false;
    postApplicationJsonInt(42, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "integer").toInt(), 42);
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, "42");
    QTRY_COMPARE_EQ(done, true);

    // NOTE: JSON string value should be framed by quotes (\"\")
    // The quotes around means this is a JSON string, not plain text.
    done = false;
    QString jsonString("\"Hello, people!\"");
    postApplicationJsonString(::OpenAPI::OptionalParam<QString>(jsonString), this,
                              [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "json-string").toString(), jsonString.mid(1, jsonString.size() - 2));
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, jsonString);
    QTRY_COMPARE_EQ(done, true);

    // NOTE: JSON supports null, see here https://spec.openapis.org/oas/v3.1.1.html#data-types
    // The argument type of postApplicationJsonString operation declared in the following way
    // [string, null] in yaml file, it means the value can be serialized as null-json: 'null'.
    done = false;
    postApplicationJsonString(::OpenAPI::OptionalParam<QString>(OptionalParam<QString>::IsNull), this,
                              [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "json-string").toString(), "null");
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, "null");
    QTRY_COMPARE_EQ(done, true);

    done = false;
    QList<OAIUser> users;
    for (qsizetype i = 0; i < 4; i++) {
        OAIUser user;
        user.setName(QString("UserName%1").arg(i));
        user.setStatus("a child");
        user.setAge(i);
        users.append(user);
    }
    // JSON Array of objects
    postApplicationJsonArray(users, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
        QJsonArray array = getJsonValue(summary, "users").toArray();
        QCOMPARE(array.size(), users.size());
        for (qsizetype i = 0; i < array.size(); i++) {
            OAIUser user;
            user.fromJsonObject(array.at(i).toObject());
            QVERIFY(users.contains(user));
        }
    });
    QCOMPARE(m_requestContent,
             "[{\"age\":0,\"name\":\"UserName0\",\"status\":\"a child\"},{\"age\":1,\"name\":\"UserName1\",\"status\":\"a child\"},{\"age\":2,\"name\":\"UserName2\",\"status\":\"a child\"},{\"age\":3,\"name\":\"UserName3\",\"status\":\"a child\"}]");
    QTRY_COMPARE_EQ(done, true);

    done = false;
    QString expectedUser("{\"First-User\":{\"age\":8778,\"name\":\"Tatiana\",\"status\":\"is working\"}}");
    QMap<QString, OAIUser> mapOfUsers;
    OAIUser mapUser;
    mapUser.setName("Tatiana");
    mapUser.setStatus("is working");
    mapUser.setAge(8778);
    mapOfUsers.insert("First-User", mapUser);
    postApplicationJsonMap(mapOfUsers, this, [&](const QRestReply &reply, const QString &summary){
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
        QCOMPARE(getJsonValue(summary, "nested-object").toJson(QJsonDocument::Compact), expectedUser);
    });
    QCOMPARE(m_requestContent, expectedUser);
    QTRY_COMPARE_EQ(done, true);

    // JSON object
    done = false;
    OAIUser user;
    user.setName("Tatiana");
    user.setStatus("is working");
    user.setAge(99);
    postApplicationJsonObject(user, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getUserByStatusObject(summary), user);
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, "{\"age\":99,\"name\":\"Tatiana\",\"status\":\"is working\"}");
    QTRY_COMPARE_EQ(done, true);

    // JSON nested object
    done = false;
    OAIUser nestedObject;
    nestedObject.setName("User Userovich");
    nestedObject.setStatus("is resting");
    nestedObject.setAge(76);
    OAIPostApplicationJsonSeveralObjects_request request;
    request.setUuid("f81d4fae-7dec-11d0-a765-00a0c91e6bf6");
    request.setUser(nestedObject);
    postApplicationJsonSeveralObjects(request, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        OAIPostApplicationJsonSeveralObjects_request response;
        response.fromJsonObject(getJsonValue(summary, "nested-object").toObject());
        QCOMPARE(response, request);
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent,
             "{\"user\":{\"age\":76,\"name\":\"User Userovich\",\"status\":\"is resting\"},\"uuid\":\"f81d4fae-7dec-11d0-a765-00a0c91e6bf6\"}");
    QTRY_COMPARE_EQ(done, true);
}

void MediaType::testPlainText_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::newRow("QString(simple-string)") << QString("I am a plain user input -_-");
    QTest::newRow("QString(simple-with-special-characters)") << QString("No encoding *+,;=!$&'()");
}

// MEDIA TYPE text/plain;
// text/plain is used with type 'string' for raw text.
// Text is treated as UTF-8 text.
void MediaType::testPlainText()
{
    QFETCH(QString, stringValue);
    bool done = false;
    // NOTE: encoding is NOT being applied for text/plain type by SPEC.
    postPlainTextType(stringValue, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary).toString(), stringValue);
        QCOMPARE(getHeaderValue(summary), "text/plain");
    });
    QCOMPARE(m_requestContent, stringValue);
    QTRY_COMPARE_EQ(done, true);
}

// MEDIA TYPE `application/octet-stream`
// It’s essentially the default "binary file" content type.
// File could be any type (e.g., .zip, .exe, .txt, etc.)
// Transmitting arbitrary byte streams.
void MediaType::testOctetStream()
{
    bool done = false;
    // We can send text file as a binary file,
    // parse it on server side, check the file content and send the string back.
    OAIHttpFileElement file;
    file.setFileName(":/file-for-uploading.txt");
    binaryType(::OpenAPI::OptionalParam<OAIHttpFileElement>(file), this,
               [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), "application/octet-stream");
        QCOMPARE(getJsonValue(summary, "file-content").toString(), "Hello world!\n");
    });
    QCOMPARE(m_requestContent, "Hello world!\n");
    QTRY_COMPARE_EQ(done, true);

    // png is from qtbase auto-tests
    done = false;
    QImage imgFromFile(":/usericon.png");
    OAIHttpFileElement icon;
    icon.setFileName(":/usericon.png");
    binaryType(::OpenAPI::OptionalParam<OAIHttpFileElement>(icon), this,
               [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), "application/octet-stream");
    });
    QImage image;
    image.loadFromData(m_requestContent, "PNG");
    QCOMPARE(image.size(), imgFromFile.size());
    QTRY_COMPARE_EQ(done, true);
}

} // OpenAPI

QTEST_MAIN(OpenAPI::MediaType)
#include "MediaType.moc"
