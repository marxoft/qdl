import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI


MySheet {
    id: root

    property bool showFiles: false
    property string startFolder: Settings.downloadPath

    signal fileChosen(string filePath)

    acceptButtonText: folderText.text == "" ? "" : qsTr("Done")
    rejectButtonText: qsTr("Cancel")
    onAccepted: root.fileChosen(root.showFiles ? fileList.chosenFile.slice(7) : folderListModel.folder.toString().slice(7))
    content: Item {
        anchors.fill: parent

        Label {
            id: folderText

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: UI.PADDING_DOUBLE
            }

            font.bold: true
            elide: Text.ElideRight
            color: "#0881cb"
            text: showFiles ? fileList.chosenFile.slice(fileList.chosenFile.lastIndexOf("/") + 1) : folderListModel.folder.toString().slice(7)
        }

        SeparatorLabel {
            id: separator

            anchors {
                left: parent.left
                right: parent.right
                top: folderText.bottom
                topMargin: UI.PADDING_DOUBLE
            }

            text: showFiles ? qsTr("Files") : qsTr("Folders")
        }

        ListView {
            id: fileList

            property string chosenFile: ""

            anchors {
                top: separator.bottom
                topMargin: UI.PADDING_DOUBLE
                left: parent.left
                right: parent.right
                bottom: backButton.top
                bottomMargin: UI.PADDING_DOUBLE
            }
            clip: true
            model: FolderListModel {
                id: folderListModel

                nameFilters: ["*.txt"]
                folder: root.startFolder
                showDotAndDotDot: false
                showFiles: root.showFiles
                showDirs: true
            }
            delegate: FileBrowserDelegate {
                id: delegate

                onClicked: folderListModel.isFolder(index) ? folderListModel.folder = filePath : fileList.chosenFile = filePath
            }
        }

        ScrollDecorator {
            flickableItem: fileList
        }

        ToolIcon {
            id: backButton

            z: 1000
            anchors {
                left: parent.left
                bottom: parent.bottom
            }
            platformIconId: "toolbar-back"
            onClicked: folderListModel.folder = folderListModel.parentFolder
        }
    }

    Label {
        id: noResultsText

        anchors.centerIn: parent
        font.pixelSize: 40
        font.bold: true
        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: qsTr("Folder empty")
        visible: fileList.count == 0
    }
}
