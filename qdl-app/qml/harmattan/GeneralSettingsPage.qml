import QtQuick 1.1
import com.nokia.meego 1.0
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

            TitleHeader {
                title: qsTr("General")
            }

            ValueListItem {
                title: qsTr("Default download path")
                subTitle: Settings.downloadPath
                enabled: TransferModel.count == 0
                onClicked: {
                    loader.sourceComponent = fileBrowserDialog;
                    loader.item.open();
                }
            }

            SwitchItem {
                title: qsTr("Start downloads automatically")
                checked: Settings.startTransfersAutomatically
                onCheckedChanged: Settings.startTransfersAutomatically = checked
            }

            SwitchItem {
                title: qsTr("Monitor clipboard for URLs")
                checked: Settings.monitorClipboard
                onCheckedChanged: Settings.monitorClipboard = checked
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }

    Loader {
        id: loader
    }

    Component {
        id: fileBrowserDialog

        FileBrowserDialog {
            onFileChosen: Settings.downloadPath = filePath
        }
    }
}
