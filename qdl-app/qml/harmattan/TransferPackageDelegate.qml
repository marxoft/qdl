import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.items 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property Transfer packageTransfer: TransferModel.get(index, -1)

    width: parent ? parent.width : screen.displayWidth
    height: column.height

    Column {
        id: column

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        TransferItemDelegate {
            parentIndex: -1
            width: parent.width
            onClicked: {
                view.selectedIndex = index;
                view.selectedParentIndex = parentIndex;
                dialogLoader.sourceComponent = transferPropertiesDialog;
                dialogLoader.item.open();
            }
            onDoubleClicked: packageTransfer.expanded = !packageTransfer.expanded
            onPressAndHold: {
                view.selectedIndex = index;
                view.selectedParentIndex = parentIndex;
                contextMenu.open();
            }
        }

        Loader {
            id: loader

            height: packageTransfer.expanded ? packageTransfer.count * 140 : 0
            width: parent.width
            sourceComponent: packageTransfer.expanded ? childComponent : undefined
        }
    }

    Component {
        id: childComponent

        Column {
            width: column.width

            Repeater {
                id: repeater

                property int parentIndex: index

                model: packageTransfer.count

                TransferItemDelegate {
                    parentIndex: repeater.parentIndex
                    width: column.width
                    onClicked: {
                        view.selectedIndex = index;
                        view.selectedParentIndex = parentIndex;
                        dialogLoader.sourceComponent = transferPropertiesDialog;
                        dialogLoader.item.open();
                    }
                    onPressAndHold: {
                        view.selectedIndex = index;
                        view.selectedParentIndex = parentIndex;
                        contextMenu.open();
                    }
                }
            }
        }
    }
}
