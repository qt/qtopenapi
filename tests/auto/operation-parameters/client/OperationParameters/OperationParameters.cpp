// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/OAITestApi.h"

#include <QtCore/qobject.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

#define CALL_TEST_OPERATION(OPERATION, PARAM, EXPECTED_STRING)                          \
{                                                                                       \
    bool done = false;                                                                  \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QString &summary) {       \
        if (!(done = reply.isSuccess()))                                                \
            qWarning() << "Error happened while issuing request : " << reply.errorString(); \
        QCOMPARE(getStatusString(summary), EXPECTED_STRING);                            \
    });                                                                                 \
    QCOMPARE("/v2" + m_testOperationPath, EXPECTED_STRING);                             \
    QTRY_COMPARE_EQ(done, true);                                                        \
}                                                                                       \

#define CALL_NOT_FOUND_TEST_OPERATION(OPERATION, PARAM)                                 \
{                                                                                       \
    bool done = true;                                                                   \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QString &summary) {       \
        Q_UNUSED(summary)                                                               \
        done = reply.isSuccess();                                                       \
        QCOMPARE(reply.httpStatus(), 404);                                              \
    });                                                                                 \
    QTRY_COMPARE_EQ(done, false);                                                       \
}                                                                                       \

namespace OpenAPI {

QString getStatusString(const QString &summary)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        return obj.value("status").toString();
    }
    return QString();
}

class OperationParameters : public OAITestApi {
    Q_OBJECT

private Q_SLOTS:
    void pathStringParameters_data();
    void pathStringParameters();
    void pathArrayParameters_data();
    void pathArrayParameters();
    void pathAnyTypeParameters_data();
    void pathAnyTypeParameters();
    void pathObjectParameters_data();
    void pathObjectParameters();
    void queryParameters();
    void queryAnyTypeParameters_data();
    void queryAnyTypeParameters();
    void queryNACombinations();
    void pathAndQueryUndefined();
};

// The latest implementation is done based on this information:
// Here is standard 3.1.1 https://spec.openapis.org/oas/v3.1.1.html#style-values
// NOTE! All QUERY and PATH params are encoded.
// https://www.speakeasy.com/openapi/requests/parameters/path-parameters#how-to-override-path-parameter-encoding
// and https://www.speakeasy.com/openapi/requests/parameters/query-parameters
void OperationParameters::pathStringParameters_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QString(Hello World!)") << QString("Hello World!")
                                           << "/v2/path/string/simple-explode/Hello%20World%21"
                                           << "/v2/path/string/simple-not-explode/Hello%20World%21"
                                           << "/v2/path/string/label-explode/.Hello%20World%21"
                                           << "/v2/path/string/label-not-explode/.Hello%20World%21"
                                           << "/v2/path/string/matrix-explode/;stringParameter=Hello%20World%21"
                                           << "/v2/path/string/matrix-not-explode/;stringParameter=Hello%20World%21";

    QTest::newRow("QString(QwerTy12399.)") << QString("QwerTy12399.")
                                           << "/v2/path/string/simple-explode/QwerTy12399."
                                           << "/v2/path/string/simple-not-explode/QwerTy12399."
                                           << "/v2/path/string/label-explode/.QwerTy12399."
                                           << "/v2/path/string/label-not-explode/.QwerTy12399."
                                           << "/v2/path/string/matrix-explode/;stringParameter=QwerTy12399."
                                           << "/v2/path/string/matrix-not-explode/;stringParameter=QwerTy12399.";
    // test sub-delims: *+,;=!$&'()
    QTest::newRow("QString(sub-delims: *+,;=!$&'())") << QString("*+,;=!$&'()")
                                                      << "/v2/path/string/simple-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/simple-not-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/label-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/label-not-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/matrix-explode/;stringParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/matrix-not-explode/;stringParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29";

}

void OperationParameters::pathStringParameters()
{
    QFETCH(QString, stringValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=string
    CALL_TEST_OPERATION(simpleExplodeString, stringValue, expectedSimpleExplode);

    // style=simple, explode=false, type=string
    CALL_TEST_OPERATION(simpleNotExplodeString, stringValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=string
    CALL_TEST_OPERATION(labelExplodeString, stringValue, expectedLabelExplode);

    // style=label, explode=false, type=string
    CALL_TEST_OPERATION(labelNotExplodeString, stringValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=string
    CALL_TEST_OPERATION(matrixExplodeString, stringValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=string
    CALL_TEST_OPERATION(matrixNotExplodeString, stringValue, expectedMatrixNotExplode);
}

void OperationParameters::pathArrayParameters_data()
{
    QTest::addColumn<QList<int>>("arrayIntValues");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QList<int>({-90, 0, 0, 2, 87867})") << QList<int>({-90, 0, 0, 2, 87867})
                                                       << "/v2/path/array/simple-explode/-90,0,0,2,87867"
                                                       << "/v2/path/array/simple-not-explode/-90,0,0,2,87867"
                                                       << "/v2/path/array/label-explode/.-90.0.0.2.87867"
                                                       << "/v2/path/array/label-not-explode/.-90,0,0,2,87867"
                                                       << "/v2/path/array/matrix-explode/;arrayParameter=-90;arrayParameter=0;arrayParameter=0;arrayParameter=2;arrayParameter=87867"
                                                       << "/v2/path/array/matrix-not-explode/;arrayParameter=-90,0,0,2,87867";
    QTest::newRow("QList<int>({1, 2, -9, 90})") << QList<int>({1, 2, -9, 90})
                                                << "/v2/path/array/simple-explode/1,2,-9,90"
                                                << "/v2/path/array/simple-not-explode/1,2,-9,90"
                                                << "/v2/path/array/label-explode/.1.2.-9.90"
                                                << "/v2/path/array/label-not-explode/.1,2,-9,90"
                                                << "/v2/path/array/matrix-explode/;arrayParameter=1;arrayParameter=2;arrayParameter=-9;arrayParameter=90"
                                                << "/v2/path/array/matrix-not-explode/;arrayParameter=1,2,-9,90";

}

void OperationParameters::pathArrayParameters()
{
    QFETCH(QList<int>, arrayIntValues);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=array
    CALL_TEST_OPERATION(simpleExplodeArray, arrayIntValues, expectedSimpleExplode);

    // style=simple, explode=false, type=array
    CALL_TEST_OPERATION(simpleNotExplodeArray, arrayIntValues, expectedSimpleNotExplode);

    // style=label, explode=true, type=array
    CALL_TEST_OPERATION(labelExplodeArray, arrayIntValues, expectedLabelExplode);

    // style=label, explode=false, type=array
    CALL_TEST_OPERATION(labelNotExplodeArray, arrayIntValues, expectedLabelNotExplode);

    // style=matrix, explode=true, type=array
    CALL_TEST_OPERATION(matrixExplodeArray, arrayIntValues, expectedMatrixExplode);

    // style=matrix, explode=false, type=array
    CALL_TEST_OPERATION(matrixNotExplodeArray, arrayIntValues,  expectedMatrixNotExplode);
}

void OperationParameters::pathAnyTypeParameters_data()
{
    OAITestObject obj;
    obj.setName("Super*+,;=!$&'()Puper");
    obj.setStatus("Awake or Not $");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QJsonValue(string)") << QJsonValue("*+,;=!$&'()John Doe")
                                        << "/v2/path/anytype/simple-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/simple-not-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/label-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/label-not-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/matrix-explode/;anytypeParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe";
    QTest::newRow("QJsonValue(int)")    << QJsonValue(100)
                                     << "/v2/path/anytype/simple-explode/100"
                                     << "/v2/path/anytype/simple-not-explode/100"
                                     << "/v2/path/anytype/label-explode/.100"
                                     << "/v2/path/anytype/label-not-explode/.100"
                                     << "/v2/path/anytype/matrix-explode/;anytypeParameter=100"
                                     << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=100";
    QTest::newRow("QJsonValue(array)")  << QJsonValue({ 1, 2.2, QString("Strange*+,;=!$&'()")})
                                       << "/v2/path/anytype/simple-explode/1,2.2,Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/simple-not-explode/1,2.2,Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/label-explode/.1.2.2.Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/label-not-explode/.1,2.2,Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/matrix-explode/;anytypeParameter=1;anytypeParameter=2.2;anytypeParameter=Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=1,2.2,Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29";
    QTest::newRow("QJsonValue(object)") << QJsonValue(obj.asJsonObject())
                                        << "/v2/path/anytype/simple-explode/name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status=Awake%20or%20Not%20%24"
                                        << "/v2/path/anytype/simple-not-explode/name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24"
                                        << "/v2/path/anytype/label-explode/.name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper.status=Awake%20or%20Not%20%24"
                                        << "/v2/path/anytype/label-not-explode/.name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24"
                                        << "/v2/path/anytype/matrix-explode/;name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper;status=Awake%20or%20Not%20%24"
                                        << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24";
}

void OperationParameters::pathAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=AnyType
    CALL_TEST_OPERATION(simpleExplodeAnytype, jsonValue, expectedSimpleExplode);

    // style=simple, explode=false, type=AnyType
    CALL_TEST_OPERATION(simpleNotExplodeAnytype, jsonValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=AnyType
    CALL_TEST_OPERATION(labelExplodeAnytype, jsonValue, expectedLabelExplode);

    // style=label, explode=false, type=AnyType
    CALL_TEST_OPERATION(labelNotExplodeAnytype, jsonValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=AnyType
    CALL_TEST_OPERATION(matrixExplodeAnytype, jsonValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=AnyType
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, jsonValue, expectedMatrixNotExplode);
}

void OperationParameters::pathObjectParameters_data()
{
    OAITestObject object;
    object.setName("Igor");
    object.setStatus("Sleep");
    QTest::addColumn<OAITestObject>("objectValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("OAITestObject({Igor, Sleep})") << object
                                                  << "/v2/path/object/simple-explode/name=Igor,status=Sleep"
                                                  << "/v2/path/object/simple-not-explode/name,Igor,status,Sleep"
                                                  << "/v2/path/object/label-explode/.name=Igor.status=Sleep"
                                                  << "/v2/path/object/label-not-explode/.name,Igor,status,Sleep"
                                                  << "/v2/path/object/matrix-explode/;name=Igor;status=Sleep"
                                                  << "/v2/path/object/matrix-not-explode/;objectParameter=name,Igor,status,Sleep";
    object.setName("TestName123");
    object.setStatus("Maybe-Awake");
    QTest::newRow("OAITestObject({TestName123, Maybe-Awake})") << object
                                                               << "/v2/path/object/simple-explode/name=TestName123,status=Maybe-Awake"
                                                               << "/v2/path/object/simple-not-explode/name,TestName123,status,Maybe-Awake"
                                                               << "/v2/path/object/label-explode/.name=TestName123.status=Maybe-Awake"
                                                               << "/v2/path/object/label-not-explode/.name,TestName123,status,Maybe-Awake"
                                                               << "/v2/path/object/matrix-explode/;name=TestName123;status=Maybe-Awake"
                                                               << "/v2/path/object/matrix-not-explode/;objectParameter=name,TestName123,status,Maybe-Awake";
    object.setName("SoMe");
    object.setStatus(" *+,;=!$&'()");
    QTest::newRow("OAITestObject({SoMe, ' *+,;=!$&'()'})") << object
                                                << "/v2/path/object/simple-explode/name=SoMe,status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/simple-not-explode/name,SoMe,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/label-explode/.name=SoMe.status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/label-not-explode/.name,SoMe,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/matrix-explode/;name=SoMe;status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/matrix-not-explode/;objectParameter=name,SoMe,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29";

}

void OperationParameters::pathObjectParameters()
{
    QFETCH(OAITestObject, objectValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=object
    CALL_TEST_OPERATION(simpleExplodeObject, objectValue, expectedSimpleExplode);

    // style=simple, explode=false, type=object
    CALL_TEST_OPERATION(simpleNotExplodeObject, objectValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=object
    CALL_TEST_OPERATION(labelExplodeObject, objectValue, expectedLabelExplode);

    // style=label, explode=false, type=object
    CALL_TEST_OPERATION(labelNotExplodeObject, objectValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=object
    CALL_TEST_OPERATION(matrixExplodeObject, objectValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=object
    CALL_TEST_OPERATION(matrixNotExplodeObject, objectValue, expectedMatrixNotExplode);
}

// The latest implementation is done based on this information:
// Here is standard 3.1.1 https://spec.openapis.org/oas/v3.1.1.html#style-values
// NOTE! We add data-driven test-cases for queries later with adding new operations in the OP.yaml.
void OperationParameters::queryParameters()
{
    // style=form, explode=true, type=array
    CALL_TEST_OPERATION(formExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                        "/v2/query/array/form-explode/formExplodeArray?arrayParameter=-90&arrayParameter=0&arrayParameter=0&arrayParameter=2&arrayParameter=87867");

    // style=form, explode=false, type=array
    CALL_TEST_OPERATION(formNotExplodeArray, QList<int>({1, 2, -9, 90}),
                        "/v2/query/array/form-not-explode/formNotExplodeArray?arrayParameter=1,2,-9,90");

    // Only style=FORM supports primitive types (string, int, double, float)
    // style=form, explode=true, type=string
    CALL_TEST_OPERATION(formExplodeString, ::OpenAPI::OptionalParam<QString>("hello, guys!"),
                        "/v2/query/string/form-explode/formExplodeString?stringParameter=hello%2C%20guys%21");

    // style=form, explode=false, type=string
    CALL_TEST_OPERATION(formNotExplodeString, ::OpenAPI::OptionalParam<QString>("hello, guys!"),
                        "/v2/query/string/form-not-explode/formNotExplodeString?stringParameter=hello%2C%20guys%21");

    // style=form, explode=true, type=object
    OAITestObject formObj;
    formObj.setName("TestName+123");
    formObj.setStatus("Awake");
    CALL_TEST_OPERATION(formExplodeObject, formObj,
                        "/v2/query/object/form-explode/formExplodeObject?name=TestName%2B123&status=Awake");

    // style=form, explode=false, type=object
    CALL_TEST_OPERATION(formNotExplodeObject, formObj,
                        "/v2/query/object/form-not-explode/formNotExplodeObject?objectParameter=name,TestName%2B123,status,Awake");

    // style=spaceDelimited, explode=false, type=array
    CALL_TEST_OPERATION(spaceDelimitedNotExplodeArray, QList<int>({1, 2, -9, 90}),
                        "/v2/query/array/spaceDelimited-not-explode/spaceDelimitedNotExplodeArray?arrayParameter=1%202%20-9%2090");

    // style=spaceDelimited, explode=false, type=object
    OAITestObject spaceDelimitedObj;
    spaceDelimitedObj.setName("TestName 123 *+,;=!$&'()");
    spaceDelimitedObj.setStatus("Awake!");
    CALL_TEST_OPERATION(spaceDelimitedNotExplodeObject, spaceDelimitedObj,
                        "/v2/query/object/spaceDelimited-not-explode/spaceDelimitedNotExplodeObject?objectParameter=name%20TestName%20123%20%2A%2B%2C%3B%3D%21%24%26%27%28%29%20status%20Awake%21");

    // Primitives are not defined for spaceDelimited and pipeDelimited, so anytype=array or anytype-object are not possible
    // style=spaceDelimited, explode=false, type=anytype array
    CALL_TEST_OPERATION(spaceDelimitedNotExplodeAnytype, QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()")}),
                        "/v2/query/anytype/spaceDelimited-not-explode/spaceDelimitedNotExplodeAnytype?anytypeParameter=1%202.2%20Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=spaceDelimited, explode=false, type=anytype object
    CALL_TEST_OPERATION(spaceDelimitedNotExplodeAnytype, QJsonValue(spaceDelimitedObj.asJsonObject()),
                        "/v2/query/anytype/spaceDelimited-not-explode/spaceDelimitedNotExplodeAnytype?anytypeParameter=name%20TestName%20123%20%2A%2B%2C%3B%3D%21%24%26%27%28%29%20status%20Awake%21");

    // style=pipeDelimited, explode=false, type=array
    CALL_TEST_OPERATION(pipeDelimitedNotExplodeArray, QList<int>({1, 2, -9, 90}),
                        "/v2/query/array/pipeDelimited-not-explode/pipeDelimitedNotExplodeArray?arrayParameter=1%7C2%7C-9%7C90");

    // style=pipeDelimited, explode=false, type=object
    OAITestObject pipeDelimitedObj;
    pipeDelimitedObj.setName("pipeDelimited=TestName");
    pipeDelimitedObj.setStatus("pipeDelimited-Sleeping *+,;=!$&'()");
    CALL_TEST_OPERATION(pipeDelimitedNotExplodeObject, pipeDelimitedObj,
                        "/v2/query/object/pipeDelimited-not-explode/pipeDelimitedNotExplodeObject?objectParameter=name%7CpipeDelimited%3DTestName%7Cstatus%7CpipeDelimited-Sleeping%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=pipeDelimited, explode=false, type=anytype array
    CALL_TEST_OPERATION(pipeDelimitedNotExplodeAnytype, QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()")}),
                        "/v2/query/anytype/pipeDelimited-not-explode/pipeDelimitedNotExplodeAnytype?anytypeParameter=1%7C2.2%7CStrange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=pipeDelimited, explode=false, type=anytype object
    CALL_TEST_OPERATION(pipeDelimitedNotExplodeAnytype, QJsonValue(pipeDelimitedObj.asJsonObject()),
                        "/v2/query/anytype/pipeDelimited-not-explode/pipeDelimitedNotExplodeAnytype?anytypeParameter=name%7CpipeDelimited%3DTestName%7Cstatus%7CpipeDelimited-Sleeping%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=deepObject, explode=true, type=object
    OAITestObject deepObjectObj;
    deepObjectObj.setName("deepObject *+,;=!$&'()-TestName");
    deepObjectObj.setStatus("deepObject-Sleeping");
    CALL_TEST_OPERATION(deepObjectExplodeObject, deepObjectObj,
                        "/v2/query/object/deepObject-explode/deepObjectExplodeObject?objectParameter%5Bname%5D=deepObject%20%2A%2B%2C%3B%3D%21%24%26%27%28%29-TestName&objectParameter%5Bstatus%5D=deepObject-Sleeping");
}

void OperationParameters::queryAnyTypeParameters_data()
{
    OAITestObject obj;
    obj.setName("Super Puper *+,;=!$&'()");
    obj.setStatus("Awake!");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedFormExplode");
    QTest::addColumn<QString>("expectedFormNotExplode");
    QTest::newRow("QJsonValue(string)") << QJsonValue("John Doe")
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=John%20Doe"
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=John%20Doe";
    QTest::newRow("QJsonValue(int)")    << QJsonValue(100)
                                     << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=100"
                                     << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=100";
    QTest::newRow("QJsonValue(array)")  << QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()") })
                                       << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=1&anytypeParameter=2.2&anytypeParameter=Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=1,2.2,Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29";
    QTest::newRow("QJsonValue(object)") << QJsonValue(obj.asJsonObject())
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?name=Super%20Puper%20%2A%2B%2C%3B%3D%21%24%26%27%28%29&status=Awake%21"
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=name,Super%20Puper%20%2A%2B%2C%3B%3D%21%24%26%27%28%29,status,Awake%21";
    QTest::newRow("QJsonValue(Null)")   << QJsonValue(QJsonValue::Null)
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
    QTest::newRow("QJsonValue(Undefined)") << QJsonValue(QJsonValue::Undefined)
                                           << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                           << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
    QTest::newRow("QJsonValue()") << QJsonValue()
                                  << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                  << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
}

// spaceDelimited, pipeDelimited, deepObject are NOT defined for Primitives.
// It means the standard doesn't say anything about those combinations. So we can skip
// spaceDelimited, pipeDelimited, deepObject cases for AnyType(primitives).
// AnyType(array) is not defined for deepObject.
// See https://spec.openapis.org/oas/v3.1.1.html#style-values
void OperationParameters::queryAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedFormExplode);
    QFETCH(QString, expectedFormNotExplode);

    // style=form, explode=true, type=anytype
    CALL_TEST_OPERATION(formExplodeAnytype, jsonValue, expectedFormExplode);
    // style=form, explode=false, type=anytype
    CALL_TEST_OPERATION(formNotExplodeAnytype, jsonValue, expectedFormNotExplode);
}

/**
 * Pay attention, the behavior of spaceDelimited, pipeDelimited and deepObject combinations
 * below are undefined!!!
 * See https://spec.openapis.org/oas/v3.1.1.html#style-values
 * But we decided to generate a warning and to fallback to the explode=false for such cases.
**/
void OperationParameters::queryNACombinations()
{
    // style=spaceDelimited, explode=true, type=array
    CALL_TEST_OPERATION(spaceDelimitedExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                        "/v2/query/array/spaceDelimited-explode/spaceDelimitedExplodeArray?arrayParameter=-90%200%200%202%2087867");

    // style=spaceDelimited, explode=true, type=empty array
    CALL_TEST_OPERATION(spaceDelimitedExplodeArray, QList<int>(),
                        "/v2/query/array/spaceDelimited-explode/spaceDelimitedExplodeArray?arrayParameter=");

    // style=pipeDelimited, explode=true, type=array
    CALL_TEST_OPERATION(pipeDelimitedExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                        "/v2/query/array/pipeDelimited-explode/pipeDelimitedExplodeArray?arrayParameter=-90%7C0%7C0%7C2%7C87867");

    // style=pipeDelimited, explode=true, type=empty array
    CALL_TEST_OPERATION(pipeDelimitedExplodeArray, QList<int>(),
                        "/v2/query/array/pipeDelimited-explode/pipeDelimitedExplodeArray?arrayParameter=");

    // style=spaceDelimited, explode=true, type=object
    OAITestObject spaceDelimitedObj;
    spaceDelimitedObj.setName("TestName123");
    spaceDelimitedObj.setStatus("Awake");
    CALL_TEST_OPERATION(spaceDelimitedExplodeObject, spaceDelimitedObj,
                        "/v2/query/object/spaceDelimited-explode/spaceDelimitedExplodeObject?objectParameter=name%20TestName123%20status%20Awake");

    // style=pipeDelimited, explode=true, type=object
    OAITestObject pipeDelimitedObj;
    pipeDelimitedObj.setName("pipeDelimited-TestName");
    pipeDelimitedObj.setStatus("pipeDelimited-Sleeping");
    CALL_TEST_OPERATION(pipeDelimitedExplodeObject, pipeDelimitedObj,
                        "/v2/query/object/pipeDelimited-explode/pipeDelimitedExplodeObject?objectParameter=name%7CpipeDelimited-TestName%7Cstatus%7CpipeDelimited-Sleeping");

    // style=deepObject, explode=false, type=object
    OAITestObject deepObjectObj;
    deepObjectObj.setName("deepObject-TestName");
    deepObjectObj.setStatus("deepObject-Sleeping");
    CALL_TEST_OPERATION(deepObjectNotExplodeObject, deepObjectObj,
                        "/v2/query/object/deepObject-not-explode/deepObjectNotExplodeObject?objectParameter%5Bname%5D=deepObject-TestName&objectParameter%5Bstatus%5D=deepObject-Sleeping");
}

void OperationParameters::pathAndQueryUndefined()
{
    QString emptyLine;
    QJsonValue nullJson(QJsonValue::Null);
    QJsonValue undefinedJson(QJsonValue::Undefined);
    QJsonValue emptyJson;

    // style=form, explode=true, type=string
    CALL_TEST_OPERATION(formExplodeString, emptyLine, "/v2/query/string/form-explode/formExplodeString?stringParameter=");

    // style=form, explode=false, type=string
    CALL_TEST_OPERATION(formNotExplodeString, emptyLine, "/v2/query/string/form-not-explode/formNotExplodeString?stringParameter=");

    // style=matrix, explode=true, type=string
    CALL_TEST_OPERATION(matrixExplodeString, emptyLine, "/v2/path/string/matrix-explode/;stringParameter");

    // style=matrix, explode=false, type=string
    CALL_TEST_OPERATION(matrixNotExplodeString, emptyLine, "/v2/path/string/matrix-not-explode/;stringParameter");

    // style=matrix, explode=true, type=AnyType(NULL)
    CALL_TEST_OPERATION(matrixExplodeAnytype, nullJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(NULL)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, nullJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // style=matrix, explode=true, type=AnyType(undefined)
    CALL_TEST_OPERATION(matrixExplodeAnytype, undefinedJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(undefined)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, undefinedJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // style=matrix, explode=true, type=AnyType(empty)
    CALL_TEST_OPERATION(matrixExplodeAnytype, emptyJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(empty)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, emptyJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // undefined case for form with empty object
    // style=form, explode=false, type=object
    CALL_TEST_OPERATION(formNotExplodeObject, OAITestObject(),
                        "/v2/query/object/form-not-explode/formNotExplodeObject?objectParameter=");

    // undefined case for form with empty object
    // style=form, explode=true, type=empty object
    CALL_TEST_OPERATION(formExplodeObject, OAITestObject(),
                        "/v2/query/object/form-explode/formExplodeObject?objectParameter=");

    // style=form, explode=true, type=empty array
    CALL_TEST_OPERATION(formExplodeArray, QList<int>(),
                        "/v2/query/array/form-explode/formExplodeArray?arrayParameter=");

    // style=form, explode=false, type=empty array
    CALL_TEST_OPERATION(formNotExplodeArray, QList<int>(),
                        "/v2/query/array/form-not-explode/formNotExplodeArray?arrayParameter=");

    // Despite the fact that https://spec.openapis.org/oas/v3.1.1.html#style-values
    // defines the undefined column for null/emptied parameters,
    // servers decline empty LABEL or SIMPLE operations like
    // "/path/string/label-not-explode/." or "/path/string/simple-explode/"
    // and return a code 404
    //
    // style=label, explode=true, type=string
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeString, emptyLine);

    // style=label, explode=false, type=string
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeString, emptyLine);

    // style=simple, explode=true, type=string
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeString, emptyLine);

    // style=simple, explode=false, type=string
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeString, emptyLine);

    // style=simple, explode=true, type=AnyType(null)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, nullJson);

    // style=simple, explode=false, type=AnyType(null)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, nullJson);

    // style=simple, explode=true, type=AnyType(undefinedJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, undefinedJson);

    // style=simple, explode=false, type=AnyType(undefinedJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, undefinedJson);

    // style=simple, explode=true, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, emptyJson);

    // style=simple, explode=false, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, emptyJson);

    // style=label, explode=true, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, undefinedJson);

    // style=label, explode=false, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, undefinedJson);

    // style=label, explode=true, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, nullJson);

    // style=label, explode=false, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, nullJson);

    // style=label, explode=true, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, emptyJson);

    // style=label, explode=false, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, emptyJson);
}

} // OpenAPI

QTEST_MAIN(OpenAPI::OperationParameters)
#include "OperationParameters.moc"
