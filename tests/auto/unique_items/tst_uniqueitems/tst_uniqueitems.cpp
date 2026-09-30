// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "uniqueitemsapi.h"

#include <QtOpenApiCommon/qoaihelpers.h>

#include <QtCore/qobject.h>
#include <QtCore/qset.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {

/**
 *
 * Array schemas with "uniqueItems: true" are generated as QSet.
 * The following test checks that the generated client code, which
 * serializes such parameters in query, path, header and cookie,
 * compiles.
 *
 * NOTE:
 * The test does not require a server side, as we are not testing
 * the data transfer here.
 *
**/
class tst_UniqueItems : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void serializeSet();
    void generatedApi();
};

void tst_UniqueItems::serializeSet()
{
    SerializationOptions opts;
    opts.style = "form";
    opts.delimiter = u","_s;

    QCOMPARE(serializeArrayValue(QSet<QString>{ u"one"_s }, opts), u"one"_s);

    const QString result = serializeArrayValue(QSet<QString>{ u"one"_s, u"two"_s }, opts);
    const QStringList items = result.split(u',');
    QCOMPARE(items.size(), 2);
    QVERIFY(items.contains(u"one"_s));
    QVERIFY(items.contains(u"two"_s));
}

void tst_UniqueItems::generatedApi()
{
    UniqueItemsApi api;

    // FIXME: QSet<Color> stays empty, as inserting into it does not compile
    // with QT_NO_SINGLE_ARGUMENT_QHASH_OVERLOAD due to the single-argument
    // qHash() overload of QOAIEnum.
#if 0
    Color red;
    red.setValue(Color::eColor::RED);
    Color blue;
    blue.setValue(Color::eColor::BLUE);
    const QSet<Color> colors{ red, blue };
#else
    const QSet<Color> colors;
#endif

    bool done = true;
    api.queryUniqueEnums(colors, QSet<QString>{ u"one"_s, u"two"_s }, this,
                         [&](const QRestReply &reply) { done = reply.isSuccess(); });
    QTRY_COMPARE_EQ(done, false);

    done = true;
    api.pathUniqueEnums(colors, this,
                        [&](const QRestReply &reply) { done = reply.isSuccess(); });
    QTRY_COMPARE_EQ(done, false);

    done = true;
    api.headerUniqueEnums(colors, this,
                          [&](const QRestReply &reply) { done = reply.isSuccess(); });
    QTRY_COMPARE_EQ(done, false);

    done = true;
    api.cookieUniqueEnums(colors, this,
                          [&](const QRestReply &reply) { done = reply.isSuccess(); });
    QTRY_COMPARE_EQ(done, false);
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::tst_UniqueItems)
#include "tst_uniqueitems.moc"
