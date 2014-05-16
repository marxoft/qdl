import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    property alias name: nameField.text
    property string path

    signal categorySet(string name, string path)

    rejectButtonText: qsTr("Cancel")
    acceptButtonText: (nameField.text == "") || (root.path == "") ? "" : qsTr("Done")
    content: Item {
        id: contentItem

        anchors.fill: parent
        clip: true

        Flickable {
            id: flicker

            anchors.fill: parent
            contentHeight: column.height + UI.PADDING_DOUBLE

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
                    text: qsTr("Name")
                    font.family: UI.FONT_FAMILY_LIGHT
                }

                TextField {
                    id: nameField

                    width: parent.width
                    platformSipAttributes: SipAttributes {
                        actionKeyEnabled: nameField.text != ""
                        actionKeyHighlighted: true
                        actionKeyLabel: qsTr("Done")
                        actionKeyIcon: ""
                    }

                    Keys.onEnterPressed: nameField.platformCloseSoftwareInputPanel()
                    Keys.onReturnPressed: nameField.platformCloseSoftwareInputPanel()
                }

                ValueListItem {
                    id: pathSelector

                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    title: qsTr("Download path")
                    subTitle: root.path == "" ? qsTr("None chosen") : root.path
                    onClicked: {
                        loader.sourceComponent = fileBrowserDialog;
                        loader.item.open();
                    }
                }
            }
        }

        ScrollDecorator {
            flickableItem: flicker
        }
    }

    onAccepted: root.categorySet(nameField.text, root.path)

    Loader {
        id: loader
    }

    Component {
        id: fileBrowserDialog

        FileBrowserDialog {
            startFolder: root.path == "" ? "/home/user/MyDocs/" : root.path
            onFileChosen: root.path = filePath
        }
    }
}
