// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

#include "QtOAIUsersApi.h"
#include "QtOAIColorsApi.h"
#include "QtOAIRegisterApi.h"

namespace QtOpenAPI {

class ColorPaletteTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialTest();
};

void ColorPaletteTests::initialTest() {
    qDebug() << "Nothing to do yet";
    QtOAIUsersApi usersApi;
    QtOAIColorsApi colorsApi;
    QtOAIRegisterApi registerApi;
}
} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::ColorPaletteTests)
#include "ColorPaletteTests.moc"
