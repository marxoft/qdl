import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.items 1.0
import com.marxoft.enums 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root

    property Transfer transfer: packageTransferModel.get(index)

    ServiceIcon {
        id: serviceIcon

        width: 48
        height: 48
        anchors {
            left: parent.left
            leftMargin: UI.PADDING_LARGE
            verticalCenter: parent.verticalCenter
        }

        iconSource: "file://" + transfer.iconFileName
        smooth: true
    }

    Label {
        id: titleLabel

        anchors {
            left: serviceIcon.right
            leftMargin: UI.PADDING_LARGE
            right: progressBar.left
            rightMargin: UI.PADDING_LARGE
            verticalCenter: parent.verticalCenter
        }

        font.bold: true
        elide: Text.ElideRight
        text: transfer.fileName
    }

    ProgressBar {
        id: progressBar

        width: 120
        anchors {
            right: parent.right
            rightMargin: UI.PADDING_LARGE
            verticalCenter: parent.verticalCenter
        }

        value: progress
        minimumValue: 0
        maximumValue: 100
        indeterminate: transfer.status === Transfers.Connecting
    }

    MouseArea {
        z: 1000
        anchors.fill: parent
    }
}
