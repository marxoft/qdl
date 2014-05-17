import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import com.marxoft.enums 1.0
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
                title: qsTr("Other")
            }

            ValueSelector {
                title: qsTr("Screen orientation")
                model: ScreenOrientationModel {}
                value: Settings.screenOrientation
                onValueChanged: Settings.screenOrientation = value
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
