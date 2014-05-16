import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property alias title: titleLabel.text
    property alias minimumValue: slider.minimumValue
    property alias maximumValue: slider.maximumValue
    property alias stepSize: slider.stepSize
    property string key

    function setKey(key, defaultValue) {
        root.key = key;
        var value = Settings.setting(key);

        if (value === undefined) {
            slider.value = parseFloat(defaultValue);
        }
        else {
            slider.value = parseFloat(value);
        }
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

    Slider {
        id: slider

        anchors {
            left: parent.left
            leftMargin: 30
            right: parent.right
            rightMargin: 30
            top: parent.verticalCenter
        }
        orientation: Qt.Horizontal
        valueIndicatorVisible: true
        onValueChanged: Settings.setSetting(root.key, slider.value)
    }
}
