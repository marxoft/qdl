import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0

ValueListItem {
    id: root

    property string key
    property variant value

    function setList(key, defaultValue, list) {
        root.key = key;

        var value = Settings.setting(key);

        if (value === undefined) {
            value = defaultValue;
            Settings.setSetting(key, value);
        }

        for (var i = 0; i < list.length; i++) {
            selectionModel.addItem(list[i].name, list[i].value);

            if (list[i].value === value) {
                root.value = value;
                root.subTitle = list[i].name;
            }
        }
    }

    onClicked: {
        loader.sourceComponent = dialog;
        loader.item.open();
    }

    SelectionModel {
        id: selectionModel
    }

    Loader {
        id: loader
    }

    Component {
        id: dialog

        ValueDialog {
            titleText: root.title
            model: selectionModel
            value: root.value
            onNameChanged: root.subTitle = name
            onValueChanged: {
                root.value = value;
                Settings.setSetting(root.key, value);
            }
        }
    }
}
