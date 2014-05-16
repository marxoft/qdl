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

        ToolIcon {
            platformIconId: "toolbar-add"
            onClicked: {
                loader.sourceComponent = editCategoryDialog;
                loader.item.name = "";
                loader.item.path = "";
                loader.item.open();
            }
        }
    }

    ContextMenu {
        id: contextMenu

        MenuLayout {

            MenuItem {
                text: qsTr("Edit")
                onClicked: {
                    loader.sourceComponent = editCategoryDialog;
                    loader.item.name = categoriesModel.data(view.selectedIndex, "name");
                    loader.item.path = categoriesModel.data(view.selectedIndex, "path");
                    loader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Remove")
                onClicked: categoriesModel.removeCategory(view.selectedIndex)
            }
        }
    }

    ListView {
        id: view

        property int selectedIndex: -1

        anchors.fill: parent
        interactive: count > 0
        model: CategoriesModel {
            id: categoriesModel
        }
        header: TitleHeader {
            title: qsTr("Categories")
        }
        delegate: CategoryDelegate {
            onClicked: {
                loader.sourceComponent = editCategoryDialog;
                loader.item.name = name;
                loader.item.path = path;
                loader.item.open();
            }
            onPressAndHold: {
                view.selectedIndex = index;
                contextMenu.open();
            }
        }

        Label {
            anchors.centerIn: view
            font {
                bold: true
                pixelSize: 40
            }
            color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            text: qsTr("No categories")
            visible: categoriesModel.count == 0
        }
    }

    ScrollDecorator {
        flickableItem: view
    }

    Loader {
        id: loader
    }

    Component {
        id: editCategoryDialog

        EditCategoryDialog {
            onCategorySet: categoriesModel.addCategory(name, path)
        }
    }
}

