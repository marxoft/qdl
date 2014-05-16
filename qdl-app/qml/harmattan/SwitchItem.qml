import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property alias title: title.text
    property alias checked: switcher.checked

    signal checkedChanged

    width: parent.width
    height: 80

    Label {
        id: title

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: switcher.left
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        font.bold: true
        verticalAlignment: Text.AlignVCenter
        maximumLineCount: 2
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
    }

    Switch {
        id: switcher

        anchors {
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }

        onCheckedChanged: root.checkedChanged()
    }
}
