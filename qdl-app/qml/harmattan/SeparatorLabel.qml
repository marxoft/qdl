import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property alias text: label.text

    height: 20

    Label {
        id: label

        anchors {
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            top: parent.top
        }
        font.pixelSize: UI.FONT_XSMALL
        font.bold: true
        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
        height: 1
        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: label.left
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: label.verticalCenter
        }

        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
    }
}
