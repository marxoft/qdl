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
            onClicked: appWindow.pageStack.pop()
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
            spacing: UI.PADDING_DOUBLE

            TitleHeader {
                title: qsTr("Interfaces")
            }

            SeparatorLabel {
                width: parent.width
                text: qsTr("Web interface")
            }

            SwitchItem {
                title: qsTr("Enable web interface")
                checked: Settings.enableWebInterface
                onCheckedChanged: Settings.enableWebInterface = checked
            }

            Row {
                x: UI.PADDING_DOUBLE
                spacing: UI.PADDING_DOUBLE

                Label {
                    width: column.width - 200 - UI.PADDING_DOUBLE * 2
                    height: portEdit.height
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    text: qsTr("Listen on port") + ":"
                }

                TextField {
                    id: portEdit

                    width: 200
                    inputMethodHints: Qt.ImhDigitsOnly
                    enabled: !Settings.enableWebInterface
                    onTextChanged: Settings.webInterfacePort = parseInt(text)
                    Component.onCompleted: text = Settings.webInterfacePort
                }
            }

            ValueSelector {
                width: parent.width
                title: qsTr("Theme")
                model: WebInterfaceThemeModel {}
                value: Settings.webInterfaceTheme
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }
}
