import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    property alias text: urlsEdit.text

    signal urlsAvailable(string urls)

    rejectButtonText: qsTr("Cancel")
    acceptButtonText: urlsEdit.text == "" ? "" : qsTr("Done")
    content: Item {
        id: contentItem

        anchors.fill: parent

        Flickable {
            id: flicker

            anchors {
                fill: parent
                margins: UI.PADDING_DOUBLE
            }
            contentHeight: urlsEdit.height + UI.PADDING_DOUBLE

            TextArea {
                id: urlsEdit

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                height: 200
                placeholderText: qsTr("Retrieve URLs")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            }
        }

        ScrollDecorator {
            flickableItem: flicker
        }
    }

    onAccepted: {
        root.urlsAvailable(urlsEdit.text);
        urlsEdit.text = "";
    }
    onRejected: urlsEdit.text = ""
}
