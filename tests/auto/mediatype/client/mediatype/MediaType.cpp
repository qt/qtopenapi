// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/QtOAITestApi.h"

#include <QtCore/qobject.h>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

namespace QtOpenAPI {
static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

QString fromFormUrlEncoding(const QString &input)
{
    // Revert (+) -> to ( )
    // Unsafe or non-alphanumeric characters → percent-encoded (%XX)
    // Alphanumeric characters (a–z, A–Z, 0–9) → left as-is
    // The following characters are always percent-encoded in x-www-form-urlencoded:
    // "!" / "$" / "&" / "'" / "(" / ")" "*" / "+" / "," / ";" / "="
    QString result(input);
    result = result.replace("+", " ");
    return QUrl::fromPercentEncoding(result.toUtf8());
}

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

QtOAIUser getUserByStatusObject(const QString &summary)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    QtOAIUser user;
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.value("json-object").isObject())
            user.fromJsonObject(obj.value("json-object").toObject());
    }
    return user;
}

class MediaType : public QtOAITestApi {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void testJsonMediaType();
    void testPlainText_data();
    void testPlainText();
    void testOctetStream();
    void testUrlEncodedType();
    void testFormMediaTypes();
    void cleanupTestCase();
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
    postApplicationJsonString(::QtOpenAPI::OptionalParam<QString>(jsonString), this,
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
    postApplicationJsonString(::QtOpenAPI::OptionalParam<QString>(OptionalParam<QString>::IsNull), this,
                              [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "json-string").toString(), "null");
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent, "null");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postApplicationJsonString(::QtOpenAPI::OptionalParam<QString>(), this,
                              [&](const QRestReply &reply, const QString &summary) {
                                  if (!(done = reply.isSuccess()))
                                      qWarning() << "ERROR: " << reply.errorString() << reply.error();\
                                  QCOMPARE(getJsonValue(summary, "json-string").toString(), "");
                                  QCOMPARE(getHeaderValue(summary), appJsonHeader);
                              });
    QCOMPARE(m_requestContent, "");
    QTRY_COMPARE_EQ(done, true);

    done = false;
    QList<QtOAIUser> users;
    for (qsizetype i = 0; i < 4; i++) {
        QtOAIUser user;
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
            QtOAIUser user;
            user.fromJsonObject(array.at(i).toObject());
            QVERIFY(users.contains(user));
        }
    });
    QCOMPARE(m_requestContent,
             "[{\"age\":0,\"name\":\"UserName0\",\"status\":\"a child\"},{\"age\":1,\"name\":\"UserName1\",\"status\":\"a child\"},{\"age\":2,\"name\":\"UserName2\",\"status\":\"a child\"},{\"age\":3,\"name\":\"UserName3\",\"status\":\"a child\"}]");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    QList<QtOAIUser> empty;
    postApplicationJsonArray(empty, this,
                             [&](const QRestReply &reply, const QString &summary) {
                                 if (!(done = reply.isSuccess()))
                                     qWarning() << "ERROR: " << reply.errorString() << reply.error();
                                 QCOMPARE(getHeaderValue(summary), appJsonHeader);
                                 QJsonArray array = getJsonValue(summary, "users").toArray();
                                 QVERIFY(array.size() == 0);
                             });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    QString expectedUser("{\"First-User\":{\"age\":8778,\"name\":\"Tatiana\",\"status\":\"is working\"}}");
    QMap<QString, QtOAIUser> mapOfUsers;
    QtOAIUser mapUser;
    mapUser.setName("Tatiana");
    mapUser.setStatus("is working");
    mapUser.setAge(8778);
    mapOfUsers.insert("First-User", mapUser);
    postApplicationJsonMap(mapOfUsers, this, [&](const QRestReply &reply, const QString &summary){
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
        const QJsonObject nestedObj = getJsonValue(summary, "nested-object").toObject();
        const QByteArray nestedJson = QJsonDocument(nestedObj).toJson(QJsonDocument::Compact);
        QCOMPARE(nestedJson, expectedUser);
    });
    QCOMPARE(m_requestContent, expectedUser);
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postApplicationJsonMap(QMap<QString, QtOAIUser>(),
                           this, [&](const QRestReply &reply, const QString &summary) {
                               if (!(done = reply.isSuccess()))
                                   qWarning() << "ERROR: " << reply.errorString() << reply.error();
                               QCOMPARE(getHeaderValue(summary), appJsonHeader);
                               QVERIFY(getJsonValue(summary, "nested-object").toObject().isEmpty());
                           });
    QTRY_COMPARE_EQ(done, true);

    // JSON object
    done = false;
    QtOAIUser user;
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

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postApplicationJsonObject(QtOAIUser(),
                              this, [&](const QRestReply &reply, const QString &summary) {
                                  if (!(done = reply.isSuccess()))
                                      qWarning() << "ERROR: " << reply.errorString() << reply.error();
                                  QCOMPARE(getHeaderValue(summary), appJsonHeader);
                                  QVERIFY(getJsonValue(summary, "json-object").toObject().isEmpty());
                              });
    QTRY_COMPARE_EQ(done, true);

    // JSON nested object
    done = false;
    QtOAIUser nestedObject;
    nestedObject.setName("User Userovich");
    nestedObject.setStatus("is resting");
    nestedObject.setAge(76);
    QtOAIPostApplicationJsonSeveralObjects_request request;
    request.setUuid("f81d4fae-7dec-11d0-a765-00a0c91e6bf6");
    request.setUser(nestedObject);
    postApplicationJsonSeveralObjects(request, this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QtOAIPostApplicationJsonSeveralObjects_request response;
        response.fromJsonObject(getJsonValue(summary, "nested-object").toObject());
        QCOMPARE(response, request);
        QCOMPARE(getHeaderValue(summary), appJsonHeader);
    });
    QCOMPARE(m_requestContent,
             "{\"user\":{\"age\":76,\"name\":\"User Userovich\",\"status\":\"is resting\"},\"uuid\":\"f81d4fae-7dec-11d0-a765-00a0c91e6bf6\"}");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postApplicationJsonSeveralObjects(QtOAIPostApplicationJsonSeveralObjects_request(),
                                      this, [&](const QRestReply &reply, const QString &summary) {
                                          if (!(done = reply.isSuccess()))
                                              qWarning() << "ERROR: " << reply.errorString() << reply.error();
                                          QCOMPARE(getHeaderValue(summary), appJsonHeader);
                                          QVERIFY(getJsonValue(summary, "nested-object").toObject().isEmpty());
                                      });
    QTRY_COMPARE_EQ(done, true);
}

void MediaType::testPlainText_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::newRow("QString(simple-string)") << QString("I am a plain user input -_-");
    QTest::newRow("QString(simple-with-special-characters)") << QString("No encoding *+,;=!$&'()");
    QTest::newRow("QString(empty-string)") << QString();
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
    QtOAIHttpFileElement file;
    file.setFileName(":/file-for-uploading.txt");
    binaryType(::QtOpenAPI::OptionalParam<QtOAIHttpFileElement>(file), this,
               [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), "application/octet-stream");
        QCOMPARE(getJsonValue(summary, "file-content").toString().trimmed(), "Hello world!");
    });
    QCOMPARE(m_requestContent.trimmed(), "Hello world!");
    QTRY_COMPARE_EQ(done, true);

    // png is from qtbase auto-tests
    done = false;
    QImage imgFromFile(":/usericon.png");
    QtOAIHttpFileElement icon;
    icon.setFileName(":/usericon.png");
    binaryType(::QtOpenAPI::OptionalParam<QtOAIHttpFileElement>(icon), this,
               [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getHeaderValue(summary), "application/octet-stream");
    });
    QImage image;
    image.loadFromData(m_requestContent, "PNG");
    QCOMPARE(image.size(), imgFromFile.size());
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    binaryType(::QtOpenAPI::OptionalParam<QtOAIHttpFileElement>(), this,
               [&](const QRestReply &reply, const QString &summary) {
                   if (!(done = reply.isSuccess()))
                       qWarning() << "ERROR: " << reply.errorString() << reply.error();
                   QCOMPARE(getHeaderValue(summary), "application/octet-stream");
               });
    QTRY_COMPARE_EQ(done, true);
}

// MEDIA TYPE `application/x-www-form-urlencoded`
// NOTE: the application/x-www-form-urlencoded type is being serialized
// as style=form, explode=true into simple key-value pairs composed by '&' and encoded.
//
// Each value from pairs of application/x-www-form-urlencoded can
// be treated based on rules, which depend on value's DATA type.
// The rules are defined here: see https://spec.openapis.org/oas/v3.1.1.html#common-fixed-fields-0
// See example of serialization + encoding here:
// https://spec.openapis.org/oas/v3.1.1.html#example-url-encoded-form-with-json-values
void MediaType::testUrlEncodedType()
{
    bool done = false;
    QtOAIUser user;
    user.setName("Lazy Cat");
    user.setStatus("Sleeping Beeping *+,;=!$&'()");
    user.setAge(101);
    QList<QString> days = {"Monday", "Sunday", "*+,;=!$&'()"};
    QMap<QString, QtOAIUser> userMap;
    userMap.insert("PET", user);
    postUrlEncodedFields(::QtOpenAPI::OptionalParam<QString>("John *+,;=!$&'()"),
                         ::QtOpenAPI::OptionalParam<qint32>(98665),
                         ::QtOpenAPI::OptionalParam<bool>(true),
                         ::QtOpenAPI::OptionalParam<QList<QString>>(days),
                         ::QtOpenAPI::OptionalParam<QMap<QString, QtOAIUser>>(userMap),
                         this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary).toVariant().toInt(), 98665);
        QCOMPARE(getJsonValue(summary, "name").toString(), "John *+,;=!$&'()");
        QCOMPARE(getJsonValue(summary, "availability").toVariant().toBool(), true);
        QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
        QtOAIUser receivedUser;
        receivedUser.fromJsonObject(getJsonValue(getJsonValue(summary, "mapfield").toString(), "PET").toObject());
        QCOMPARE(receivedUser, user);
        QJsonArray array = getJsonValue(summary, "visits").toArray();
        for (qsizetype i = 0; i < array.size(); i++) {
            const QString day = array.at(i).toString();
            QVERIFY(days.contains(day));
        }
    });
    QCOMPARE(m_requestContent,
             "name=John+%2A%2B%2C%3B%3D%21%24%26%27%28%29&status=98665&availability=true&visits=Monday&visits=Sunday&visits=%2A%2B%2C%3B%3D%21%24%26%27%28%29&mapfield=%7B%22PET%22%3A%7B%22age%22%3A101%2C%22name%22%3A%22Lazy+Cat%22%2C%22status%22%3A%22Sleeping+Beeping+%2A%2B%2C%3B%3D%21%24%26%27%28%29%22%7D%7D");
    QCOMPARE(fromFormUrlEncoding(m_requestContent),
             "name=John *+,;=!$&'()&status=98665&availability=true&visits=Monday&visits=Sunday&visits=*+,;=!$&'()&mapfield={\"PET\":{\"age\":101,\"name\":\"Lazy Cat\",\"status\":\"Sleeping Beeping *+,;=!$&'()\"}}");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postUrlEncodedFields(::QtOpenAPI::OptionalParam<QString>(),
                         ::QtOpenAPI::OptionalParam<qint32>(),
                         ::QtOpenAPI::OptionalParam<bool>(),
                         ::QtOpenAPI::OptionalParam<QList<QString>>(),
                         ::QtOpenAPI::OptionalParam<QMap<QString, QtOAIUser>>(),
                         this, [&](const QRestReply &reply, const QString &summary) {
                             if (!(done = reply.isSuccess()))
                                 qWarning() << "ERROR: " << reply.errorString() << reply.error();

                             QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
                         });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    // NOTE: - 'user' field is being serialized as application/json.
    // NOTE: - 'comment' field is being serialized as text/plain.
    // Each field of application/x-www-form-urlencoded object are
    // being treated based on rules, which depend on a field DATA type. See example:
    // https://spec.openapis.org/oas/v3.1.1.html#example-url-encoded-form-with-json-values
    QtOAIUser enUrlUser;
    enUrlUser.setName("Tatiana");
    enUrlUser.setStatus("is working");
    enUrlUser.setAge(100);
    postUrlEncodedNestedObject(enUrlUser, ::QtOpenAPI::OptionalParam<QString>("Test String "),
                               this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QtOAIUser received;
        received.fromJson(getJsonValue(summary, "user").toString());
        QCOMPARE(received, enUrlUser);
        QCOMPARE(getJsonValue(summary, "comment").toString(), "Test String ");
        QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
    });
    QCOMPARE(m_requestContent, "user=%7B%22age%22%3A100%2C%22name%22%3A%22Tatiana%22%2C%22status%22%3A%22is+working%22%7D&comment=Test+String+");
    QCOMPARE(fromFormUrlEncoding(m_requestContent), "user={\"age\":100,\"name\":\"Tatiana\",\"status\":\"is working\"}&comment=Test String ");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    QtOAIUser emptyUser;
    postUrlEncodedNestedObject(emptyUser, ::QtOpenAPI::OptionalParam<QString>(),
                               this, [&](const QRestReply &reply, const QString &summary) {
                                   if (!(done = reply.isSuccess()))
                                       qWarning() << "ERROR: " << reply.errorString() << reply.error();
                                   QtOAIUser received;
                                   received.fromJson(getJsonValue(summary, "user").toString());
                                   QCOMPARE(received, emptyUser);
                                   QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
                               });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    postUrlEncodedObject(::QtOpenAPI::OptionalParam<QString>("User Name 1234 "),
                         ::QtOpenAPI::OptionalParam<QString>("Thinking"),
                         ::QtOpenAPI::OptionalParam<qint32>(8776513),
                         this, [&](const QRestReply &reply, const QString &summary) {
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary).toString(), "Thinking");
        QCOMPARE(getJsonValue(summary, "username").toString(), "User Name 1234 ");
        QCOMPARE(getJsonValue(summary, "age").toVariant().toInt(), 8776513);
        QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
    });
    QCOMPARE(m_requestContent, "name=User+Name+1234+&status=Thinking&age=8776513");
    QCOMPARE(fromFormUrlEncoding(m_requestContent), "name=User Name 1234 &status=Thinking&age=8776513");
    QTRY_COMPARE_EQ(done, true);

    // EMPTY requestContent, but header still needs to be sent.
    done = false;
    postUrlEncodedObject(::QtOpenAPI::OptionalParam<QString>(),
                         ::QtOpenAPI::OptionalParam<QString>(),
                         ::QtOpenAPI::OptionalParam<qint32>(),
                         this, [&](const QRestReply &reply, const QString &summary) {
                             if (!(done = reply.isSuccess()))
                                 qWarning() << "ERROR: " << reply.errorString() << reply.error();
                             QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");
                         });
    QTRY_COMPARE_EQ(done, true);
}

// Main idea of multipart/form-data is an each field are being serialized
// and encoded differentlly, depending on content + encoding + data type combinations.
//
// Below is an example of Basic Multi-part form, see:
// https://spec.openapis.org/oas/v3.1.1.html#example-basic-multipart-form
// The example uses DEFAULT content types for form-fields.
// By default, content type depends on the DATA type of the field.
// See the table of default types under this chapter (column 'Default MediaType'):
// https://spec.openapis.org/oas/v3.1.1.html#common-fixed-fields-0
void MediaType::testFormMediaTypes()
{
    bool done = false;
    QtOAIUser user1, user2;
    user1.setName("User_1");
    user1.setStatus("Awaik");
    user1.setAge(10);
    user2.setName("User_2");
    user2.setStatus("Sleeping");
    user2.setAge(11);

    QtOAIPostMultiPartData_request_formObject object;
    object.setObjectId(-99);
    object.setObjectName("AnObject 123");

    QList<QtOAIUser> multiList = {user1, user2};
    QMap<QString, QtOAIUser> map;
    map.insert("TEXT", user1);

    QtOAIHttpFileElement formFile;
    formFile.setFileName(":/file-for-uploading.txt");
    formFile.setRequestFileName(":/file-for-uploading.txt");

    // 'formId'           - has a string type with uuid format, so it's being treated as 'plain/text'.
    //                      However we set custom `application/json` for this parameter in yaml file.
    // 'formAddresses'    - default type for arrays is based on the type in the `items` subschema.
    //                      Here it's an Object, so it is being treated as `application/json`.
    // 'formIndex'        - has a simple integer type, so it's being treated as 'plain/text'.
    // 'formProfileImage' - has a string type with binary format, so it's being treated as `application/octet-stream`.
    // 'formObject'       - default type for an Object is `application/json`.
    // 'formMap'          - default type for a Map is `application/json`.
    // NOTE: 'formId' and 'formAddresses' are declared as required in YAML file.
    postMultiPartData(QString("\"f81d4fae-7dec-11d0-a765-00a0c91e6bf6\""), // json string
                      multiList,
                      ::QtOpenAPI::OptionalParam<qint32>(100),
                      ::QtOpenAPI::OptionalParam<QtOAIHttpFileElement>(formFile),
                      ::QtOpenAPI::OptionalParam<QtOAIPostMultiPartData_request_formObject>(object),
                      ::QtOpenAPI::OptionalParam<QMap<QString, QtOAIUser>>(map),
                      this, [&](const QRestReply &reply, const QString &summary){
        if (!(done = reply.isSuccess()))
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
        QCOMPARE(getJsonValue(summary, "formId").toString(), "\"f81d4fae-7dec-11d0-a765-00a0c91e6bf6\"");
        QJsonArray array = getJsonValue(summary, "formAddresses").toArray();
        QVERIFY(array.size() > 0);
        QCOMPARE(array.at(0).toString(), "[{\"age\":10,\"name\":\"User_1\",\"status\":\"Awaik\"},{\"age\":11,\"name\":\"User_2\",\"status\":\"Sleeping\"}]");
        QCOMPARE(getJsonValue(summary, "formIndex").toVariant().toInt(), 100);
        QCOMPARE(getJsonValue(summary, "formObject").toString(), "{\"objectId\":-99,\"objectName\":\"AnObject 123\"}");
        QCOMPARE(getJsonValue(summary, "formMap").toString(), "{\"TEXT\":{\"age\":10,\"name\":\"User_1\",\"status\":\"Awaik\"}}");
        QCOMPARE(getJsonValue(summary, "formProfileImage").toString().trimmed() , "Hello world!");
        QVERIFY(getHeaderValue(summary).contains("multipart/form-data; boundary="));
    });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    postMultiPartData(QString("\"\""), // json string
                      QList<QtOAIUser>(),
                      ::QtOpenAPI::OptionalParam<qint32>(),
                      ::QtOpenAPI::OptionalParam<QtOAIHttpFileElement>(),
                      ::QtOpenAPI::OptionalParam<QtOAIPostMultiPartData_request_formObject>(),
                      ::QtOpenAPI::OptionalParam<QMap<QString, QtOAIUser>>(),
                      this, [&](const QRestReply &reply, const QString &summary) {
                          if (!(done = reply.isSuccess()))
                              qWarning() << "ERROR: " << reply.errorString() << reply.error();
                          QVERIFY(getHeaderValue(summary).contains("multipart/form-data; boundary="));
                      });
    QTRY_COMPARE_EQ(done, true);
}

void MediaType::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::MediaType)
#include "MediaType.moc"
