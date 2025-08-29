// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

namespace QtOpenAPI {

class ColorPaletteTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialTest();
};

void ColorPaletteTests::initialTest() {
    qDebug() << "Nothing to do yet";
}
} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::ColorPaletteTests)
#include "ColorPaletteTests.moc"

