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
        model: [ qsTr("General"), qsTr("Archives"), qsTr("Network"), qsTr("Interfaces"), qsTr("Categories"), qsTr("Accounts"), qsTr("Plugins"), qsTr("Other") ]

        header: TitleHeader {
            title: qsTr("Settings")
        }
        delegate: SettingsDelegate {
            onClicked: {
                switch (index) {
                case 0:
                    appWindow.pageStack.push(Qt.resolvedUrl("GeneralSettingsPage.qml"));
                    return;
                case 1:
                    appWindow.pageStack.push(Qt.resolvedUrl("ArchiveSettingsPage.qml"));
                    return;
                case 2:
                    appWindow.pageStack.push(Qt.resolvedUrl("NetworkSettingsPage.qml"));
                    return;
                case 3:
                    appWindow.pageStack.push(Qt.resolvedUrl("InterfacesSettingsPage.qml"));
                    return;
                case 4:
                    appWindow.pageStack.push(Qt.resolvedUrl("CategoriesPage.qml"));
                    return;
                case 5:
                    appWindow.pageStack.push(Qt.resolvedUrl("AccountsPage.qml"));
                    return;
                case 6:
                    appWindow.pageStack.push(Qt.resolvedUrl("PluginsSettingsPage.qml"));
                    return;
                case 7:
                    appWindow.pageStack.push(Qt.resolvedUrl("OtherSettingsPage.qml"));
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
