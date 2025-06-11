// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import client

Item {
    id: root

    property dataOAITag tagModel;
    property dataOAIOrder orderModel;
    property dataOAIApiResponse responseModel;
    property dataOAICategory categoryModel;
    property dataOAIUser userModel;
    property dataOAIPet petModel;
    property bool petAdded: false;

    OAIUserApi { id: userId }
    OAIStoreApi { id: storeId }
    OAIPetApi {
        id: petIdElement
        Component.onCompleted: {
            petIdElement.setUsername("User1");
            petIdElement.setPassword("1234");
            petIdElement.setApiKey("api_key", "special-key");
        }
    }

    Connections {
        target: petIdElement
        function onAddPetFinished(summary) {
            root.petAdded = true;
        }
        function onAddPetErrorOccurred() {
             root.petAdded = false;
        }
    }

    Timer {
        id: timer
        repeat: false
        interval: 1000
        onTriggered: testCaseId.when = true;
    }

    TestCase {
        name: "basicRegistrationTest"

        function initTestCase() {
            root.petModel.getId = 111
            root.categoryModel.getId = 123
            root.petModel.getName = "ChonkyChonk"
            root.petModel.getStatus = "Nice Cat"
            root.categoryModel.getName = "Stray Cat"
            root.petModel.getCategory = root.categoryModel
        }

        function test_setGadgetData() {
            compare(root.petModel.getId, 111)
            compare(root.categoryModel.getId, 123)
            compare(root.categoryModel.getName, "Stray Cat")
            compare(root.petModel.getName, "ChonkyChonk")
            compare(root.petModel.getStatus, "Nice Cat")
            compare(root.petModel.getCategory.getName, "Stray Cat")
        }

        function test_addPet() {
            petIdElement.addPet(root.petModel);
            timer.start();
        }
    }

    TestCase {
        id: testCaseId
        name: "callOperationTest"
        when: false
        function test_serverReply() {
            compare(true, root.petAdded);
        }
    }
}
