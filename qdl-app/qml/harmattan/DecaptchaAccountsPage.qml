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

    ContextMenu {
        id: contextMenu

        MenuLayout {

            MenuItem {
                text: qsTr("Edit")
                onClicked: {
                    loader.sourceComponent = editAccountDialog;
                    loader.item.serviceName = accountsModel.data(view.selectedIndex, "serviceName");
                    loader.item.serviceIcon = "file://" + accountsModel.data(view.selectedIndex, "serviceIcon");
                    loader.item.username = accountsModel.data(view.selectedIndex, "username");
                    loader.item.password = accountsModel.data(view.selectedIndex, "password");
                    loader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Remove")
                onClicked: accountsModel.removeAccount(view.selectedIndex)
            }
        }
    }

    ListView {
        id: view

        property int selectedIndex: -1

        anchors.fill: parent
        interactive: count > 0
        model: DecaptchaAccountsModel {
            id: accountsModel
        }
        header: TitleHeader {
            title: qsTr("Decaptcha Accounts")
        }
        delegate: AccountDelegate {
            id: delegate

            onClicked: {
                loader.sourceComponent = editAccountDialog;
                loader.item.serviceName = serviceName;
                loader.item.serviceIcon = "file://" + serviceIcon;
                loader.item.username = username;
                loader.item.password = password;
                loader.item.open();
            }
            onPressAndHold: {
                view.selectedIndex = index;
                contextMenu.open();
            }
        }

        Label {
            anchors.centerIn: view
            font {
                bold: true
                pixelSize: 40
            }
            color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            text: qsTr("No accounts")
            visible: accountsModel.count == 0
        }
    }

    ScrollDecorator {
        flickableItem: view
    }

    Loader {
        id: loader
    }

    Component {
        id: editAccountDialog

        EditAccountDialog {
            isDecaptchaAccount: true
            onAccountSet: accountsModel.addAccount(serviceName, username, password)
        }
    }
}
