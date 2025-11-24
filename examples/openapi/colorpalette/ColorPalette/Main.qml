// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Color Palette")

    ColorView {
        id: colorview
        anchors.fill: parent
        visible: true
    }
}
