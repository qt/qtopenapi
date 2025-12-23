// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "client1testapi.h"
#include "client2testapi.h"

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {

/**
 *
 * The following test checks if several separate client libraries
 * can use common resources by linking them to libraries and
 * the resulting executable file.
 *
 * The test uses an executable binary and 3 libraries:
 * cpp-qt-test-config1-app, TestClient1, TestClient2, TestCommon.
 *
 * NOTE:
 * The test does not require a server side, as we are not testing
 * the data transfer here.
 * Another reason is servers don't support several yaml files
 * generation. It means we will have to manually create a lot of code
 * on the server side.
 * But since the data transferring is not a point of the current
 * test, it would be preferable to skip such steps for now.
 *
 * The first test client is generated with following oiptions:
 * --additional-properties=...,commonLibrary=Use-Common-Lib,commonLibraryName=TestCommon --package-name=TestClient1
 * It means the Qt6 OpenAPI generator generates 2 separate libraries:
 * - TestClient1 library
 * - TestCommon library
 *
 * The second test client is generated with following oiptions:
 * --additional-properties=...,commonLibrary=Skip-Common-Files --package-name=TestClient2
 *
 * It means the Qt6 OpenAPI generator generates the following library:
 * - TestClient2 library
 *
 * For receiving access to common resources, like QOAIBaseApi
 * or QOAIHttpRequestInput classes, the TestClient2 library
 * links to the TestCommon library.
 *
 * See in the CMakeLists.txt:
 * target_link_libraries(TestClient2
 *    PRIVATE
 *        TestCommon
 * )
 *
 * Such generation configuration protects us from a possible
 * "multiple defined symbols" problem.
 *
 * The resulting test application links all 3 libraries,
 * creates objects of those libraries, and calls APIs of
 * TestClient2 and TestClient1.
 *
**/
class TestConfiguration1 : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testLibraries();
};

void TestConfiguration1::testLibraries()
{
    Client1TestApi api1;
    Client2TestApi api2;
    QOAIHttpFileElement object("NoName.txt");
    bool done = true;

    api1.simpleExplodeString("Test String"_L1, this,
                             [&](const QRestReply &reply, const QString &) {
                                 done = reply.isSuccess();
                             });

    QTRY_COMPARE_EQ(done, false);

    done = true;
    api2.simpleExplodeInt(12345, this,
                          [&](const QRestReply &reply, const QString &) {
                                 done = reply.isSuccess();
                          });
    QTRY_COMPARE_EQ(done, false);
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::TestConfiguration1)
#include "TestConfiguration1.moc"
