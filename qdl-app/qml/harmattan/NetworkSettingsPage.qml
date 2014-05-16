import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Page {
    id: root

    orientationLock: Settings.screenOrientation
    tools: ToolBarLayout {

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: {
                Settings.setNetworkProxy();
                appWindow.pageStack.pop();
            }
        }
    }

    Flickable {
        id: flicker

        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            TitleHeader {
                title: qsTr("Network")
            }

            SwitchItem {
                id: proxySwitch

                title: qsTr("Use network proxy")
                checked: Settings.networkProxyHostName != ""
                onCheckedChanged: {
                    if (!checked) {
                        Settings.networkProxyHostName = "";
                        Settings.networkProxyPort = 80;
                        Settings.networkProxyUser = "";
                        Settings.networkProxyPassword = "";
                        hostEdit.text = "";
                        portEdit.text = 80;
                        usernameEdit.text = "";
                        passwordEdit.text = "";
                    }
                }
            }

            Column {
                id: column2

                x: UI.PADDING_DOUBLE
                width: column.width - UI.PADDING_DOUBLE * 2
                opacity: proxySwitch.checked ? 1 : 0
                visible: opacity > 0
                spacing: UI.PADDING_DOUBLE

                Behavior on opacity { NumberAnimation { duration: 200 } }

                ValueSelector {
                    id: proxyTypeSelector

                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    title: qsTr("Proxy type")
                    model: NetworkProxyTypeModel {}
                    value: Settings.networkProxyType
                }

                Row {
                    spacing: parent.width - (children[0].width + children[1].width)

                    Label {
                        width: hostEdit.width
                        text: qsTr("Host")
                    }

                    Label {
                        width: portEdit.width
                        text: qsTr("Port")
                    }
                }

                Row {
                    spacing: parent.width - (children[0].width + children[1].width)

                    TextField {
                        id: hostEdit

                        width: column2.width - 230
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                        platformSipAttributes: SipAttributes {
                            actionKeyEnabled: hostEdit.text != ""
                            actionKeyHighlighted: true
                            actionKeyLabel: qsTr("Next")
                            actionKeyIcon: ""
                        }
                        onTextChanged: Settings.networkProxyHostName = text
                        Keys.onReturnPressed: portEdit.focus = true
                        Component.onCompleted: text = Settings.networkProxyHostName
                    }

                    TextField {
                        id: portEdit

                        width: 200
                        inputMethodHints: Qt.ImhDigitsOnly
                        onTextChanged: Settings.networkProxyPort = parseInt(text)
                        Component.onCompleted: text = Settings.networkProxyPort
                    }
                }

                Label {
                    text: qsTr("Username")
                }

                TextField {
                    id: usernameEdit

                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    platformSipAttributes: SipAttributes {
                        actionKeyEnabled: usernameEdit.text != ""
                        actionKeyHighlighted: true
                        actionKeyLabel: qsTr("Next")
                        actionKeyIcon: ""
                    }
                    onTextChanged: Settings.networkProxyUser = text
                    Keys.onReturnPressed: passwordEdit.focus = true
                    Component.onCompleted: text = Settings.networkProxyUser
                }

                Label {
                    text: qsTr("Password")
                }

                TextField {
                    id: passwordEdit

                    width: parent.width
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    Component.onCompleted: text = Settings.networkProxyPassword
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }
}
