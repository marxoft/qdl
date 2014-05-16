import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root

    Image {
        id: icon

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        source: parent.ListView.view.model.isFolder(index) ? "image://theme/icon-m-toolbar-directory-white" : "image://theme/icon-m-toolbar-attachment-white"
    }

    Label {
        anchors {
            left: icon.right
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        font.bold: true
        elide: Text.ElideRight
        text: fileName
    }
}
