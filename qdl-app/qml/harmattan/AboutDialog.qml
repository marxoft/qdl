import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Dialog {
    id: root

    width: parent.width
    content: Item {
        height: column.height
        anchors {
            left: parent.left
            right: parent.right
            margins: UI.PADDING_DOUBLE
        }

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: UI.PADDING_DOUBLE

            Image {
                id: icon

                x: Math.floor((parent.width / 2) - (width / 2))
                source: "images/qdl.png"
            }

            Label {
                id: titleLabel

                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                font.pixelSize: 32
                text: "QDL " + versionNumber
            }

            Label {
                id: aboutLabel

                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("A user-friendly download manager.<br><br>&copy; Stuart Howarth 2012-2014<br>")
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: "<font color='white'>" + qsTr("Contact") + ": </font><a href='mailto:showarth@marxoft.co.uk?subject=QDL " + versionNumber + " for N9'>showarth@marxoft.co.uk</a>"
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
}
