import QtQuick 1.1
import com.marxoft.items 1.0

MaskedItem {
    id: root

    property alias iconSource: icon.source

    width: 64
    height: 64
    mask: Image {
        width: root.width
        height: root.height
        source: "images/avatar-mask.png"
        fillMode: Image.Stretch
        smooth: true
    }

    Image {
        id: frame

        anchors.fill: parent
        source: "images/avatar-frame.png"
        sourceSize.width: width
        sourceSize.height: height
        smooth: true
        fillMode: Image.Stretch
        visible: icon.status == Image.Ready

        Image {
            id: icon

            z: -1
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            smooth: true
            fillMode: Image.PreserveAspectCrop
            asynchronous: false
            clip: true
            onStatusChanged: if (status == Image.Error) source = "images/qdl.png";
        }
    }
}
