// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

namespace OpenAPI {

class ColorPaletteTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialTest();
};

void ColorPaletteTests::initialTest() {
    qDebug() << "Nothing to do yet";
}
} // OpenAPI

QTEST_MAIN(OpenAPI::ColorPaletteTests)
#include "ColorPaletteTests.moc"

