import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0

Page {
    id: root

    orientationLock: Settings.screenOrientation
    tools: ToolBarLayout {

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: appWindow.pageStack.pop()
        }
    }

    ListView {
        id: view

        anchors.fill: parent
        model: [ qsTr("Service accounts"), qsTr("Decaptcha accounts") ]
        header: TitleHeader {
            title: qsTr("Accounts")
        }
        delegate: SettingsDelegate {
            onClicked: {
                switch (index) {
                case 0:
                    appWindow.pageStack.push(Qt.resolvedUrl("ServiceAccountsPage.qml"));
                    return;
                case 1:
                    appWindow.pageStack.push(Qt.resolvedUrl("DecaptchaAccountsPage.qml"));
                    return;
                default:
                    return;
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: view
    }
}
