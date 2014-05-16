import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Page {
    id: root

    orientationLock: Settings.screenOrientation
    tools: ToolBarLayout {

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: appWindow.pageStack.pop()
        }
    }

    Flickable {
        id: flicker

        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            TitleHeader {
                title: qsTr("Archives")
            }

            SwitchItem {
                title: qsTr("Extract downloaded archives")
                checked: Settings.extractDownloadedArchives
                onCheckedChanged: Settings.extractDownloadedArchives = checked
            }

            SwitchItem {
                title: qsTr("Create subfolders for archives")
                checked: Settings.createSubfolderForArchives
                enabled: Settings.extractDownloadedArchives
                onCheckedChanged: Settings.createSubfolderForArchives = checked
            }

            SwitchItem {
                title: qsTr("Delete extracted archives")
                checked: Settings.deleteExtractedArchives
                enabled: Settings.extractDownloadedArchives
                onCheckedChanged: Settings.deleteExtractedArchives = checked
            }

            SeparatorLabel {
                height: 50
                width: parent.width
                text: qsTr("Archive passwords")
            }

            Row {
                x: UI.PADDING_DOUBLE
                width: parent.width - UI.PADDING_DOUBLE * 2
                height: passwordEdit.height + UI.PADDING_DOUBLE
                spacing: UI.PADDING_DOUBLE

                TextField {
                    id: passwordEdit

                    width: parent.width - addButton.width - UI.PADDING_DOUBLE
                    placeholderText: qsTr("Add password")
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    platformSipAttributes: SipAttributes {
                        actionKeyEnabled: passwordEdit.text != ""
                        actionKeyHighlighted: true
                        actionKeyLabel: qsTr("Add")
                        actionKeyIcon: ""
                    }
                    Keys.onEnterPressed: {
                        passwordsModel.addPassword(passwordEdit.text);
                        passwordEdit.platformCloseSoftwareInputPanel();
                        passwordEdit.text = "";
                    }
                    Keys.onReturnPressed: {
                        passwordsModel.addPassword(passwordEdit.text);
                        passwordEdit.platformCloseSoftwareInputPanel();
                        passwordEdit.text = "";
                    }
                }

                Button {
                    id: addButton

                    width: height
                    iconSource: "image://theme/icon-m-toolbar-add" + (theme.inverted ? "-white" : "")
                    enabled: passwordEdit.text != ""
                    onClicked: {
                        passwordsModel.addPassword(passwordEdit.text);
                        passwordEdit.text = "";
                    }
                }
            }

            Repeater {
                id: repeater

                property int selectedIndex: -1

                model: ArchivePasswordsModel {
                    id: passwordsModel
                }

                ListItem {

                    Label {
                        anchors {
                            left: parent.left
                            leftMargin: UI.PADDING_DOUBLE
                            right: parent.right
                            rightMargin: UI.PADDING_DOUBLE
                            verticalCenter: parent.verticalCenter
                        }
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        font.bold: true
                        text: display
                    }

                    onPressAndHold: {
                        repeater.selectedIndex = index;
                        contextMenu.open();
                    }
                }
            }
        }
    }

    ContextMenu {
        id: contextMenu

        MenuLayout {

            MenuItem {
                text: qsTr("Remove")
                onClicked: passwordsModel.removePassword(repeater.selectedIndex)
            }
        }
    }
}
