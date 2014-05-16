import QtQuick 1.1

SwitchItem {
    id: root

    property string key

    function setKey(key, defaultValue) {
        root.key = key;

        var value = Settings.setting(key);

        if (value === undefined) {
            root.checked = (defaultValue === "true");
        }
        else {
            root.checked = (value === "true");
        }
    }

    onCheckedChanged: Settings.setSetting(root.key, root.checked)
}
