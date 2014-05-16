import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.items 1.0
import com.marxoft.enums 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root

    property int parentIndex: -1
    property Transfer transfer: TransferModel.get(index, parentIndex)

    height: 140

    Component {
        id: topDecoration

        Item {
            z: 100
            width: 30
            height: 85
            anchors {
                left: parent.left
                leftMargin: UI.PADDING_LARGE
                top: parent.top
                topMargin: 55
            }

            Rectangle {
                id: indicator

                width: 30
                height: 30
                color: "transparent"
                border {
                    width: 1
                    color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
                }

                Rectangle {
                    width: 2
                    anchors {
                        left: parent.left
                        leftMargin: 14
                        top: parent.top
                        topMargin: 5
                        bottom: parent.bottom
                        bottomMargin: 5
                    }
                    color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
                    visible: !packageTransfer.expanded
                }

                Rectangle {
                    height: 2
                    anchors {
                        left: parent.left
                        leftMargin: 5
                        right: parent.right
                        rightMargin: 5
                        top: parent.top
                        topMargin: 14
                    }
                    color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
                }

                MouseArea {
                    z: 100
                    width: 50
                    height: 50
                    anchors.centerIn: parent
                    onClicked: root.doubleClicked()
                }
            }

            Rectangle {
                width: 1
                anchors {
                    top: indicator.bottom
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin: 15
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
                visible: packageTransfer.expanded
            }
        }
    }

    Component {
        id: middleDecoration

        Item {
            z: 100
            width: 45
            height: root.height
            anchors {
                left: parent.left
                leftMargin: UI.PADDING_LARGE
                top: parent.top
            }

            Rectangle {
                width: 1
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin: 15
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }

            Rectangle {
                width: 25
                height: 1
                anchors {
                    top: parent.verticalCenter
                    left: parent.left
                    leftMargin: 15
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }
        }
    }

    Component {
        id: bottomDecoration

        Item {
            z: 100
            width: 45
            height: root.height
            anchors {
                left: parent.left
                leftMargin: UI.PADDING_LARGE
                top: parent.top
            }

            Rectangle {
                width: 1
                anchors {
                    top: parent.top
                    bottom: parent.verticalCenter
                    left: parent.left
                    leftMargin: 15
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }

            Rectangle {
                width: 25
                height: 1
                anchors {
                    top: parent.verticalCenter
                    left: parent.left
                    leftMargin: 15
                }
                color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            }
        }
    }

    Loader {
        id: loader

        z: 100
        sourceComponent: packageTransfer.count === 0 ? undefined : parentIndex === -1 ? topDecoration : index === (packageTransfer.count - 1) ? bottomDecoration : middleDecoration
    }

    Item {
        id: content

        anchors {
            fill: parent
            leftMargin: !loader.item ? UI.PADDING_LARGE : loader.item.width + UI.PADDING_LARGE * 2
            rightMargin: UI.PADDING_LARGE
            topMargin: UI.PADDING_LARGE
            bottomMargin: UI.PADDING_LARGE
        }

        ServiceIcon {
            id: serviceIcon

            width: 48
            height: 48
            anchors {
                top: parent.top
                left: parent.left
            }
            iconSource: "file://" + transfer.iconFileName
            smooth: true
        }

        Label {
            id: titleLabel

            anchors {
                top: parent.top
                left: serviceIcon.right
                leftMargin: UI.PADDING_LARGE
                right: parent.right
            }
            font.bold: true
            elide: Text.ElideRight
            text: transfer.fileName
        }

        ProgressBar {
            id: progressBar

            width: 150
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            value: transfer.progress
            minimumValue: 0
            maximumValue: 100
            indeterminate: transfer.status === Transfers.Connecting
        }

        Label {
            id: statusLabel

            anchors {
                left: parent.left
                right: progressLabel.visible ? progressLabel.left : parent.right
                rightMargin: progressLabel.visible ? UI.PADDING_LARGE : 0
                bottom: parent.bottom
            }
            font.pixelSize: UI.FONT_SMALL
            font.family: UI.FONT_FAMILY_LIGHT
            elide: Text.ElideRight
            text: transfer.statusString
            color: transfer.status === Transfers.Failed ? "red" : UI.COLOR_INVERTED_FOREGROUND
        }

        Label {
            id: progressLabel

            anchors {
                right: parent.right
                bottom: parent.bottom
            }
            font.pixelSize: UI.FONT_SMALL
            font.family: UI.FONT_FAMILY_LIGHT
            visible: transfer.status !== Transfers.Failed
            horizontalAlignment: Text.AlignRight
            text: Utils.fileSizeFromBytes(transfer.position) + " " + qsTr("of") + " " + (transfer.size > 0 ? Utils.fileSizeFromBytes(transfer.size) + " (" + transfer.progress + "%)" : qsTr("Unknown"))
        }
    }
}
