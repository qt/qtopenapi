// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/TestApi.h"

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtCommonOpenAPI;

namespace QtOpenAPI {

class tst_Compression : public TestApi {
    Q_OBJECT

private Q_SLOTS:
    void localCompressionRoundtrip_data();
    void localCompressionRoundtrip();
};

void tst_Compression::localCompressionRoundtrip_data()
{
    using namespace QOAIHttpRequestWorker;
    QTest::addColumn<QOAICompressionType>("compressionType");

    QTest::newRow("gzip") << QOAICompressionType::Gzip;
    QTest::newRow("deflate") << QOAICompressionType::Deflate;
}

void tst_Compression::localCompressionRoundtrip()
{
    // Generate the data to compress
    static constexpr int BlockSize = 1024;
    static constexpr size_t MaxUChar = std::numeric_limits<uchar>::max();

    QByteArray originalData;
    originalData.reserve(MaxUChar * BlockSize);
    for (uchar c = 0; c < MaxUChar; ++c)
        originalData.append(BlockSize, c);

    using namespace QOAIHttpRequestWorker;
    QFETCH(const QOAICompressionType, compressionType);

    const QByteArray compressed = compressData(originalData, 9, compressionType);
    QCOMPARE_LE(compressed.size(), originalData.size());

    const QByteArray decompressed = decompressData(compressed, compressionType);

    QCOMPARE(decompressed, originalData);
}

} // namespace QtOpenAPI

QTEST_MAIN(QtOpenAPI::tst_Compression)
#include "tst_compression.moc"
