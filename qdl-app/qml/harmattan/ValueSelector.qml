import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ValueListItem {
    id: root

    property variant value
    property variant model

    onClicked: {
        loader.sourceComponent = dialog;
        loader.item.open();
    }

    Loader {
        id: loader
    }

    Component {
        id: dialog

        ValueDialog {
            titleText: root.title
            model: root.model
            value: root.value
            onNameChanged: root.subTitle = name
            onValueChanged: root.value = value
        }
    }

    Component.onCompleted: {
        if ((value) && (model)) {
            for (var i = 0; i < model.count; i++) {
                if (model.data(i, "value") === value) {
                    subTitle = model.data(i, "name");

                    return;
                }
            }
        }
    }
}
