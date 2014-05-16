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

                Label {
                    width: parent.width
                    font.bold: true
                    text: transfer === null ? "" : transfer.packageName
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

                SeparatorLabel {
                    width: parent.width
                    text: qsTr("Files")
                }

                Repeater {
                    id: repeater

                    model: PackageTransferModel {
                        id: packageTransferModel

                        onCountChanged: if (count == 0) root.close();
                    }

                    PackageTransferDelegate {
                        x: -UI.PADDING_DOUBLE
                        width: column.width + UI.PADDING_DOUBLE * 2
                    }
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
                        enabled: transfer !== null
                        onClicked: transfer.queuePackage()
                    }

                    Button {
                        text: qsTr("Pause")
                        enabled: transfer !== null
                        onClicked: transfer.pausePackage()
                    }

                    Button {
                        text: qsTr("Remove")
                        onClicked: transfer.cancelPackage()
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
            packageTransferModel.package = transfer;
            prioritySelector.value = transfer.priority;
        }
    }
}
