import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import com.marxoft.items 1.0
import com.marxoft.enums 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    property Transfer transfer: null

    acceptButtonText: qsTr("Done")
    content: Item {
        id: contentItem

        anchors.fill: parent
        clip: true

        Flickable {
            id: flicker

            anchors.fill: parent
            contentHeight: column.height + UI.PADDING_DOUBLE * 2

            Column {
                id: column

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: UI.PADDING_DOUBLE
                }
                spacing: UI.PADDING_DOUBLE

                Row {
                    spacing: UI.PADDING_DOUBLE

                    ServiceIcon {
                        width: 48
                        height: 48
                        iconSource: transfer === null ? "" : "file://" + transfer.iconFileName
                        smooth: true
                    }

                    Label {
                        width: column.width - 48 - UI.PADDING_DOUBLE
                        height: 48
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                        text: transfer === null ? "" : transfer.fileName
                    }
                }

                TextField {
                    width: parent.width
                    readOnly: true
                    text: transfer === null ? "" : transfer.url
                    onTextChanged: cursorPosition = 0
                }

                CheckBox {
                    id: audioCheckbox

                    width: parent.width
                    text: qsTr("Convert to audio file")
                    visible: (transfer !== null) && (transfer.convertibleToAudio)
                    checked: (transfer !== null) && (transfer.convertToAudio)
                    onClicked: transfer.convertToAudio = checked
                }

                ValueSelector {
                    id: connectionsSelector

                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    title: qsTr("Connections")
                    subTitle: transfer === null ? "" : transfer.preferredConnections
                    model: SelectionModel {}
                    onValueChanged: if (transfer) transfer.preferredConnections = value;
                }

                ValueSelector {
                    id: categorySelector

                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    title: qsTr("Category")
                    subTitle: transfer === null ? "" : transfer.category
                    model: SelectionModel {}
                    onValueChanged: if (transfer) transfer.category = value;
                    Component.onCompleted: {
                        var categories = Database.getCategoryNames();

                        for (var i = 0; i < categories.length; i++) {
                            model.addItem(categories[i], categories[i]);
                        }

                        value = Settings.defaultCategory;
                    }
                }

                ValueSelector {
                    id: prioritySelector

                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    title: qsTr("Priority")
                    subTitle: transfer === null ? "" : transfer.priorityString
                    model: TransferPriorityModel {}
                    onValueChanged: if (transfer) transfer.priority = value;
                }

                Label {
                    id: progressLabel

                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: UI.FONT_SMALL
                    font.family: UI.FONT_FAMILY_LIGHT
                    text: transfer === null ? "" : Utils.fileSizeFromBytes(transfer.position) + " " + qsTr("of") + " " + (transfer.size > 0 ? Utils.fileSizeFromBytes(transfer.size) + " (" + transfer.progress + "%)" : qsTr("Unknown"))
                }

                ProgressBar {
                    id: progressBar

                    width: parent.width
                    minimumValue: 0
                    maximumValue: 100
                    value: transfer === null ? 0 : transfer.progress
                    indeterminate: (transfer !== null) && (transfer.status === Transfers.Connecting)
                }

                Label {
                    id: statusLabel

                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: UI.FONT_SMALL
                    font.family: UI.FONT_FAMILY_LIGHT
                    color: (transfer !== null) && (transfer.status === Transfers.Failed) ? "red" : UI.COLOR_INVERTED_FOREGROUND
                    text: transfer === null ? "" : transfer.statusString
                }

                SeparatorLabel {
                    x: -UI.PADDING_DOUBLE
                    width: parent.width + UI.PADDING_DOUBLE * 2
                    height: 50
                    text: qsTr("Actions")
                }

                ButtonRow {
                    exclusive: false

                    Button {
                        text: qsTr("Start")
                        enabled: (transfer !== null) && ((transfer.status === Transfers.Paused) || (transfer.status === Transfers.Failed))
                        onClicked: transfer.queue()
                    }

                    Button {
                        text: qsTr("Pause")
                        enabled: (transfer !== null) && ((transfer.status !== Transfers.Paused) && (transfer.status !== Transfers.Failed) && (transfer.status !== Transfers.Extracting) && (transfer.status !== Transfers.Converting))
                        onClicked: transfer.pause()
                    }

                    Button {
                        text: qsTr("Remove")
                        onClicked: transfer.cancel()
                    }
                }
            }
        }

        ScrollDecorator {
            flickableItem: flicker
        }
    }

    onTransferChanged: {
        if (transfer) {
            prioritySelector.value = transfer.priority;
            connectionsSelector.model.clear();

            for (var i = 1; i <= transfer.maximumConnections; i++) {
                connectionsSelector.model.addItem(i.toString(), i);
            }

            connectionsSelector.value = transfer.preferredConnections;
        }
    }

    Connections {
        target: transfer === null ? null : transfer
        onStatusChanged: {
            switch (transfer.status) {
            case Transfers.Cancelled:
            case Transfers.Completed:
                root.close();
                return;
            default:
                return;
            }
        }
    }
}
