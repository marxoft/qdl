import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    function close() {
        root.visible = false;
        root.state = "";
        UrlCheckModel.clear();
    }

    ListView {
        id: view

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: column.top
            margins: UI.PADDING_DOUBLE
        }

        model: UrlCheckModel

        header: Item {
            width: view.width
            height: 48

            Label {
                id: urlLabel

                anchors {
                    left: parent.left
                    leftMargin: UI.PADDING_LARGE
                    verticalCenter: parent.verticalCenter
                }

                verticalAlignment: Text.AlignVCenter
                text: qsTr("URL")
            }

            Label {
                id: okLabel

                anchors {
                    right: parent.right
                    rightMargin: UI.PADDING_LARGE
                    verticalCenter: parent.verticalCenter
                }

                verticalAlignment: Text.AlignVCenter
                text: qsTr("OK?")
            }

            Rectangle {
                height: 1
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }

            Rectangle {
                width: 1
                anchors {
                    top: parent.top
                    right: okLabel.left
                    bottom: parent.bottom
                    margins: UI.PADDING_LARGE
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }
        }

        delegate: UrlCheckDelegate {}
    }

    ScrollDecorator {
        flickableItem: view
    }

    Column {
        id: column

        spacing: UI.PADDING_DOUBLE
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: UI.PADDING_DOUBLE
        }

        ProgressBar {
            id: progressBar

            width: parent.width
            value: UrlChecker.progress
            maximumValue: 100
        }

        Label {
            id: label

            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: UI.FONT_SMALL
            font.italic: true
            text: qsTr("Checking URLs")
        }

        Button {
            id: button

            x: Math.floor((parent.width / 2) - (width / 2))
            text: qsTr("Cancel")
            onClicked: root.state == "" ? UrlChecker.cancel() : root.close()
        }
    }

    states: [
        State {
            name: "canceled"
            PropertyChanges { target: label; text: qsTr("Canceled") }
            PropertyChanges { target: button; text: qsTr("Close") }
        },

        State {
            name: "completed"
            when: UrlChecker.progress == 100
            PropertyChanges { target: label; text: qsTr("Completed") }
            PropertyChanges { target: button; text: qsTr("Close") }
        }
    ]

    Connections {
        target: UrlChecker
        onCanceled: root.state = "canceled"
    }
}
