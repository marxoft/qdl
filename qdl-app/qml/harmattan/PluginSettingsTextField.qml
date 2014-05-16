import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property alias title: titleLabel.text
    property string key

    function setKey(key, defaultValue) {
        root.key = key;
        var value = Settings.setting(key);

        if (value === undefined) {
            value = defaultValue;
        }

        textField.text = value;
    }

    height: 80
    width: !parent ? implicitWidth : parent.width

    Label {
        id: titleLabel

        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            top: parent.top
        }

        font.bold: true
        elide: Text.ElideRight
    }

    TextField {
        id: textField

        anchors {
            left: parent.left
            leftMargin: 30
            right: parent.right
            rightMargin: 30
            top: titleLabel.bottom
            topMargin: UI.PADDING_DOUBLE
        }

        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        platformSipAttributes: SipAttributes {
            actionKeyEnabled: textField.text != ""
            actionKeyHighlighted: true
            actionKeyLabel: qsTr("Done")
            actionKeyIcon: ""
        }

        Keys.onEnterPressed: textField.platformCloseSoftwareInputPanel()
        Keys.onReturnPressed: textField.platformCloseSoftwareInputPanel()

        onTextChanged: Settings.setSetting(root.key, textField.text)
    }
}
