import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    property alias text: urlsEdit.text

    signal urlsAvailable(string urls, string service)

    rejectButtonText: qsTr("Cancel")
    acceptButtonText: urlsEdit.text == "" ? "" : qsTr("Done")
    content: Item {
        id: contentItem

        anchors.fill: parent

        Flickable {
            id: flicker

            anchors.fill: parent
            contentHeight: column.height + UI.PADDING_DOUBLE

            Column {
                id: column

                anchors {
                    top: parent.top
                    topMargin: UI.PADDING_DOUBLE
                    left: parent.left
                    right: parent.right
                }
                spacing: UI.PADDING_DOUBLE

                TextArea {
                    id: urlsEdit

                    x: UI.PADDING_DOUBLE
                    width: parent.width - UI.PADDING_DOUBLE * 2
                    height: 200
                    placeholderText: qsTr("Add URLs")
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                }

                ValueSelector {
                    id: serviceSelector

                    width: parent.width
                    title: qsTr("Service")
                    subTitle: qsTr("Detect from URL")
                    model: SelectionModel {}
                    Component.onCompleted: {
                        model.addItem(qsTr("Detect from URL"), "");
                        var services = PluginManager.servicePluginNames();

                        for (var i = 0; i < services.length; i++) {
                            model.addItem(services[i], services[i]);
                        }

                        value = "";
                    }
                }

                ValueSelector {
                    id: categorySelector

                    width: parent.width
                    title: qsTr("Category")
                    subTitle: Settings.defaultCategory
                    model: SelectionModel {}
                    onValueChanged: Settings.defaultCategory = value
                    Component.onCompleted: {
                        var categories = Database.getCategoryNames();

                        for (var i = 0; i < categories.length; i++) {
                            model.addItem(categories[i], categories[i]);
                        }

                        value = Settings.defaultCategory;
                    }
                }
            }
        }

        ScrollDecorator {
            flickableItem: flicker
        }
    }

    onAccepted: {
        root.urlsAvailable(urlsEdit.text, serviceSelector.value);
        urlsEdit.text = "";
    }
    onRejected: urlsEdit.text = ""
}
