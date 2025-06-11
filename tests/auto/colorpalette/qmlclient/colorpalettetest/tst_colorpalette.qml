// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import client

TestCase {
    id: root

    name: "basicRegistrationTest"

    property dataOAIColor colorModel;
    property dataOAITestObject testObjectModel;
    property dataOAIUser userModel;

    function initTestCase() {
        root.colorModel.getId = 111
        root.testObjectModel.getId = 123
        root.userModel.getFirstName = "UserFirstName"
    }

    function test_setGadgetData() {
        compare(root.colorModel.getId, 111)
        compare(root.testObjectModel.getId, 123)
        compare(root.userModel.getFirstName, "UserFirstName")
    }
}
