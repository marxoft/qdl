import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    property alias serviceName: serviceLabel.text
    property alias serviceIcon: serviceIcon.iconSource
    property alias username: usernameField.text
    property alias password: passwordField.text
    property bool isDecaptchaAccount: false

    signal accountSet(string serviceName, string username, string password)

    rejectButtonText: qsTr("Cancel")
    acceptButtonText: (usernameField.text == "") || (passwordField.text == "") ? "" : qsTr("Done")
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

                Row {
                    width: parent.width
                    spacing: UI.PADDING_DOUBLE

                    ServiceIcon {
                        id: serviceIcon

                        width: 64
                        height: 64
                    }

                    Label {
                        id: serviceLabel

                        height: 64
                        width: parent.width - serviceIcon.width - UI.PADDING_DOUBLE
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        font.bold: true
                    }
                }

                Label {
                    text: qsTr("Username") + "/" + qsTr("email")
                    font.family: UI.FONT_FAMILY_LIGHT
                }

                TextField {
                    id: usernameField

                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    platformSipAttributes: SipAttributes {
                        actionKeyEnabled: root.username != ""
                        actionKeyHighlighted: true
                        actionKeyLabel: qsTr("Next")
                        actionKeyIcon: ""
                    }

                    Keys.onEnterPressed: passwordField.focus = true
                    Keys.onReturnPressed: passwordField.focus = true
                }

                Label {
                    text: qsTr("Password")
                    font.family: UI.FONT_FAMILY_LIGHT
                }

                TextField {
                    id: passwordField

                    width: parent.width
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    platformSipAttributes: SipAttributes {
                        actionKeyEnabled: (root.username != "") && (root.password != "")
                        actionKeyHighlighted: true
                        actionKeyLabel: qsTr("Done")
                        actionKeyIcon: ""
                    }

                    Keys.onEnterPressed: root.accept()
                    Keys.onReturnPressed: root.accept()
                }

                CheckBox {
                    id: checkbox

                    text: qsTr("Use this decaptcha service")
                    visible: root.isDecaptchaAccount
                    enabled: (root.username != "") && (root.password != "")
                    checked: Settings.decaptchaService == root.serviceName
                    onClicked: Settings.decaptchaService = checkbox.checked ? root.serviceName : ""
                }
            }
        }

        ScrollDecorator {
            flickableItem: flicker
        }
    }

    onAccepted: root.accountSet(root.serviceName, root.username, root.password)
}
