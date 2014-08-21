import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    width: parent.width
    height: 56

    Label {
        id: urlLabel

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_LARGE
            right: icon.left
            rightMargin: UI.PADDING_LARGE
            verticalCenter: parent.verticalCenter
        }
        elide: Text.ElideRight
        text: url
    }

    Image {
        id: icon

        width: 40
        height: 40
        anchors {
            right: parent.right
            rightMargin: UI.PADDING_LARGE
            verticalCenter: parent.verticalCenter
        }
        source: !checked ? "" : ok ? "image://theme/icon-m-toolbar-done" + (theme.inverted ? "-white" : "")
                                   : "image://theme/icon-m-toolbar-cancle" + (theme.inverted ? "-white" : "")
    }
}
