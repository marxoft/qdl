import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property alias title: title.text

    width: parent === null ? implicitWidth : parent.width
    height: 72

    Label {
        id: title

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 32
        elide: Text.ElideRight
    }

    Rectangle {
        height: 1
        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            bottom: parent.bottom
        }
        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
    }
}
