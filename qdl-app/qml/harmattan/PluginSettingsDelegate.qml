import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root

    subItemIndicator: true

    ServiceIcon {
        id: icon

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }
        iconSource: "file://" + pluginIcon
    }

    Label {
        id: title

        anchors {
            left: icon.right
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        font.bold: true
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: pluginName
    }
}
