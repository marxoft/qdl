import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root

    property bool subItemIndicator: false

    signal clicked
    signal doubleClicked
    signal pressAndHold

    height: 84
    width: parent ? parent.width : screen.displayWidth
    opacity: enabled ? UI.OPACITY_ENABLED : UI.OPACITY_DISABLED

    Loader {
        anchors {
            fill: parent
            topMargin: 1
            bottomMargin: 1
        }
        sourceComponent: mouseArea.pressed ? highlight : undefined
    }

    Component {
        id: highlight

        Rectangle {
            anchors.fill: parent
            color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
            opacity: 0.5
        }
    }

    Loader {
        anchors {
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }
        sourceComponent: root.subItemIndicator ? indicator : undefined
    }

    Component {
        id: indicator

        Image {
            source: "image://theme/icon-m-common-drilldown-arrow" + (theme.inverted ? "-inverse" : "")
        }
    }

    MouseArea {
        id: mouseArea

        z: 1
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
        onDoubleClicked: root.doubleClicked()
        onPressAndHold: root.pressAndHold()
    }
}
