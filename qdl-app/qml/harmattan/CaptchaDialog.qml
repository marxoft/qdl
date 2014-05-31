import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Window {
    id: window

    property alias captchaFileName: image.source
    property int timeOut: 0

    signal captchaResponseReady(string response)
    signal rejected

    StatusBar {
        id: statusBar

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
    }

    Sheet {
        id: sheet

        anchors {
            left: parent.left
            right: parent.right
            top: statusBar.bottom
            bottom: parent.bottom
        }

        rejectButtonText: qsTr("Cancel")
        acceptButtonText: textField.text == "" ? "" : qsTr("Done")
        content: Item {
            id: contentItem

            anchors.fill: parent
            clip: true

            Flickable {
                id: flicker

                anchors.fill: parent
                contentHeight: column.height + UI.PADDING_DOUBLE * 2

                Column {
                    id: column

                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        margins: UI.PADDING_DOUBLE
                    }
                    spacing: UI.PADDING_DOUBLE

                    Label {
                        width: parent.width
                        text: qsTr("Please complete captcha")
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Image {
                        id: image

                        x: Math.floor((parent.width / 2) - 200)
                        width: 400
                        height: 200
                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Label {
                        x: Math.floor((parent.width / 2) - 200)
                        font.family: UI.FONT_FAMILY_LIGHT
                        text: Utils.durationFromSecs(window.timeOut)
                        color: window.timeOut < 10 ? "red" : "white"
                    }

                    TextField {
                        id: textField

                        x: Math.floor((parent.width / 2) - 200)
                        width: 400
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                        platformSipAttributes: SipAttributes {
                            actionKeyEnabled: textField.text != ""
                            actionKeyHighlighted: true
                            actionKeyLabel: qsTr("Done")
                            actionKeyIcon: ""
                        }

                        Keys.onEnterPressed: sheet.accept()
                        Keys.onReturnPressed: sheet.accept()
                    }
                }
            }

            ScrollDecorator {
                flickableItem: flicker
            }
        }

        onAccepted: window.captchaResponseReady(textField.text)
        onRejected: window.rejected()
    }

    Timer {
        id: timer

        running: window.timeOut > 0
        repeat: true
        interval: 1000
        onTriggered: window.timeOut--
    }

    onTimeOutChanged: if (window.timeOut <= 0) sheet.reject();

    Component.onCompleted: {
        theme.inverted = true;
        sheet.open();
    }
}
