import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    height: 100

    function open(message, numberOfOperations) {
        progressBar.value = 0;
        progressBar.maximumValue = numberOfOperations;
        label.text = message;
        root.visible = true;
    }

    function close() {
        root.visible = false;
        progressBar.value = 0;
        progressBar.maximumValue = 0;
    }

    function updateProgress(progress) {
        progressBar.value = progress;
    }

    Label {
        id: title

        anchors {
            left: parent.left;
            right: parent.right
            top: parent.top
        }

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        font.bold: true
        text: qsTr("Please wait")
    }

    ProgressBar {
        id: progressBar

        anchors {
            left: parent.left;
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        value: 0
        maximumValue: 0
    }

    Label {
        id: label

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        elide: Text.ElideRight
        font.family: UI.FONT_FAMILY_LIGHT
        font.pixelSize: UI.FONT_SMALL
        font.italic: true
    }
}
