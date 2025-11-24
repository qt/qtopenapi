// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ColorPalette

pragma ComponentBehavior: Bound

Item {
    id: root

    property int currentColorPage: 1
    property int totalColorPages: 0

    ListModel {
        id: colorListModel
    }

    function fetchColors(page) {
        colorListModel.clear()
        ColorsApi.getColors(page)
    }

    Connections {
        target: ColorsApi

        function onGetColorsFinished(summary) { // summary is the QtOAIColorPage
            for (var i = 0; i < summary.getData.length; i++) {
                colorListModel.append({
                    id: summary.getData[i].getId,
                    name: summary.getData[i].getName,
                    color: summary.getData[i].getColor,
                    pantone_value: summary.getData[i].getPantoneValue
                });
                root.currentColorPage = summary.getPage
                root.totalColorPages = summary.getTotalPages
            }
        }

        function onGetColorsErrorOccurred(errorType, errorStr) {
            console.warn("Error message:", errorStr);
            root.resetState();
            connectionErrorPopup.open()
        }
    }

    // load colors for first page
    Component.onCompleted: fetchColors(root.currentColorPage)

    onCurrentColorPageChanged: fetchColors(currentColorPage)

    ColumnLayout {
        // The main application layout
        anchors.fill :parent

        ListView {
            id: colorListView
            model: colorListModel

            footerPositioning: ListView.OverlayFooter
            spacing: 15
            clip: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            header:  Rectangle {
                height: 32
                width: parent.width
                color: "#F0F1F3"

                RowLayout {
                    anchors.fill: parent

                    component HeaderText : Text {
                        Layout.alignment: Qt.AlignVCenter
                        horizontalAlignment: Qt.AlignHCenter

                        font.pixelSize: 12
                        color: "#667085"
                    }
                    HeaderText {
                        id: headerName
                        text: qsTr("Color Name")
                        Layout.preferredWidth: colorListView.width * 0.3
                    }
                    HeaderText {
                        id: headerRgb
                        text: qsTr("Rgb Value")
                        Layout.preferredWidth: colorListView.width * 0.25
                    }
                    HeaderText {
                        id: headerPantone
                        text: qsTr("Pantone Value")
                        Layout.preferredWidth: colorListView.width * 0.25
                    }
                    HeaderText {
                        id: headerAction
                        text: qsTr("Action")
                        Layout.preferredWidth: colorListView.width * 0.2
                    }
                }
            }

            delegate: Item {
                id: colorInfo

                required property var modelData

                width: colorListView.width
                height: 25
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5

                    Rectangle {
                        id: colorSample
                        Layout.alignment: Qt.AlignVCenter
                        implicitWidth: 36
                        implicitHeight: 21
                        radius: 6
                        color: colorInfo.modelData.color
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.3 - colorSample.width
                        horizontalAlignment: Qt.AlignLeft
                        leftPadding: 5
                        text: colorInfo.modelData.name
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.25
                        horizontalAlignment: Qt.AlignHCenter
                        text: colorInfo.modelData.color
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.25
                        horizontalAlignment: Qt.AlignHCenter
                        text: colorInfo.modelData.pantone_value
                    }
                }
            }

            footer: ToolBar {
                // Paginate buttons if more than one page
                visible: root.totalColorPages > 1
                implicitWidth: parent.width

                RowLayout {
                    anchors.fill: parent

                    Item { Layout.fillWidth: true /* spacer */ }

                    Repeater {
                        model: root.totalColorPages

                        ToolButton {
                            text: page
                            font.bold: root.currentColorPage === page

                            required property int index
                            readonly property int page: (index + 1)

                            onClicked: root.currentColorPage = page
                        }
                    }
                }
            }
        }
    }

    Popup {
            id: connectionErrorPopup
            padding: 10
            modal: true
            focus: true
            anchors.centerIn: parent
            closePolicy: Popup.CloseOnEscape

            background: Rectangle {
                color: "#F2F3F5"
                radius: 20
                border.color: "#000000"
                border.width: 1
            }

            ColumnLayout {
                Layout.preferredWidth: 280

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Connection Failure!")
                    font.pixelSize: 20
                    font.bold: true
                    color: "#F44336"
                    Layout.topMargin: 10
                }

                Label {
                    text: qsTr("The application could not retrieve data from the server.")
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 20
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    text: qsTr("Please check your server connection.")
                    font.pixelSize: 12
                    font.bold: true
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    text: qsTr("Try Again")
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 15
                    onClicked: {
                        connectionErrorPopup.close()
                        root.fetchColors(root.currentColorPage)
                    }
                }
            }
    }

    function resetState() {
        console.log("Resetting application state due to server disconnection/issue.");

        colorListModel.clear();

        root.currentColorPage = 1;
        root.totalColorPages = 0;
    }
}
