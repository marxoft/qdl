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
            onClicked: pageStack.pop()
        }
    }

    ListView {
        id: view

        anchors.fill: parent
        interactive: count > 0
        model: PluginSettingsModel {
            id: settingsModel
        }
        header: TitleHeader {
            title: qsTr("Plugins")
        }
        delegate: PluginSettingsDelegate {
            id: delegate

            onClicked: appWindow.pageStack.push(Qt.resolvedUrl("PluginSettingsPage.qml")).loadSettings(pluginName, "file://" + pluginIcon, "file://" + fileName)
        }

        Label {
            anchors.centerIn: view
            font {
                bold: true
                pixelSize: 40
            }
            color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            text: qsTr("No plugins")
            visible: settingsModel.count == 0
        }
    }

    ScrollDecorator {
        flickableItem: view
    }
}
