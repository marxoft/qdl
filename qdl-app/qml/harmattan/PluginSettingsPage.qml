import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Page {
    id: root

    function loadSettings(serviceName, serviceIcon, fileName) {
        header.title = serviceName;

        var request = new XMLHttpRequest();
        request.onreadystatechange = function() {
                    if (request.readyState === XMLHttpRequest.DONE) {
                        var doc = request.responseXML.documentElement;

                        for (var i = 0; i < doc.childNodes.length; i++) {
                            var node = doc.childNodes[i];

                            if (node.nodeName === "group") {
                                root.addGroup(node.attributes[0].value);

                                for (var ii = 0; ii < node.childNodes.length; ii++) {
                                    var groupNode = node.childNodes[ii];

                                    if (groupNode.nodeName === "list") {
                                        var key = serviceName + "/" + root.findAttributeValue(groupNode, "key");
                                        var defaultValue = root.findAttributeValue(groupNode, "default");
                                        var title = root.findAttributeValue(groupNode, "title");
                                        var list = [];

                                        for (var iii = 0; iii < groupNode.childNodes.length; iii++) {
                                            var listNode = groupNode.childNodes[iii];

                                            if (listNode.nodeName === "element") {
                                                var name = root.findAttributeValue(listNode, "name");
                                                var value = root.findAttributeValue(listNode, "value");
                                                var element = { "name": name, "value": value };
                                                list.push(element);
                                            }
                                        }

                                        root.addSelectionItem(key, defaultValue, title, list);
                                    }
                                    else if (groupNode.nodeName === "boolean") {
                                        var key = serviceName + "/" + root.findAttributeValue(groupNode, "key");
                                        var defaultValue = root.findAttributeValue(groupNode, "default");
                                        var title = root.findAttributeValue(groupNode, "title");
                                        root.addSwitch(key, defaultValue, title);
                                    }
                                    else if (groupNode.nodeName === "integer") {
                                        var key = serviceName + "/" + root.findAttributeValue(groupNode, "key");
                                        var defaultValue = root.findAttributeValue(groupNode, "default");
                                        var title = root.findAttributeValue(groupNode, "title");
                                        var min = parseInt(root.findAttributeValue(groupNode, "min"));
                                        var max = parseInt(root.findAttributeValue(groupNode, "max"));
                                        var step = parseInt(root.findAttributeValue(groupNode, "step"));
                                        root.addSlider(key, defaultValue, title, min, max, step);
                                    }
                                    else if (groupNode.nodeName === "text") {
                                        var key = serviceName + "/" + root.findAttributeValue(groupNode, "key");
                                        var defaultValue = root.findAttributeValue(groupNode, "default");
                                        var title = root.findAttributeValue(groupNode, "title");
                                        root.addTextField(key, defaultValue, title);
                                    }
                                }
                            }
                            if (node.nodeName === "list") {
                                var key = serviceName + "/" + root.findAttributeValue(node, "key");
                                var defaultValue = root.findAttributeValue(node, "default");
                                var title = root.findAttributeValue(node, "title");
                                var list = [];

                                for (var iii = 0; iii < node.childNodes.length; iii++) {
                                    var listNode = node.childNodes[iii];

                                    if (listNode.nodeName === "element") {
                                        var name = root.findAttributeValue(listNode, "name");
                                        var value = root.findAttributeValue(listNode, "value");
                                        var element = { "name": name, "value": value };
                                        list.push(element);
                                    }
                                }

                                root.addSelectionItem(key, defaultValue, title, list);
                            }
                            else if (node.nodeName === "boolean") {
                                var key = serviceName + "/" + root.findAttributeValue(node, "key");
                                var defaultValue = root.findAttributeValue(node, "default");
                                var title = root.findAttributeValue(node, "title");
                                root.addSwitch(key, defaultValue, title);
                            }
                            else if (node.nodeName === "integer") {
                                var key = serviceName + "/" + root.findAttributeValue(node, "key");
                                var defaultValue = root.findAttributeValue(node, "default");
                                var title = root.findAttributeValue(node, "title");
                                var min = parseInt(root.findAttributeValue(node, "min"));
                                var max = parseInt(root.findAttributeValue(node, "max"));
                                var step = parseInt(root.findAttributeValue(node, "step"));
                                root.addSlider(key, defaultValue, title, min, max, step);
                            }
                            else if (node.nodeName === "text") {
                                var key = serviceName + "/" + root.findAttributeValue(node, "key");
                                var defaultValue = root.findAttributeValue(node, "default");
                                var title = root.findAttributeValue(node, "title");
                                root.addTextField(key, defaultValue, title);
                            }
                        }
                    }
                }

        request.open("GET", fileName);
        request.send();
    }

    function findAttributeValue(node, name) {
        for (var i = 0; i < node.attributes.length; i++) {
            var att = node.attributes[i];

            if (att.name === name) {
                return att.value;
            }
        }

        return "";
    }

    function findEmptyLoader() {
        for (var i = 1; i < column.children.length; i++) {
            var child = column.children[i];

            if ((child.hasOwnProperty("item")) && (!child.item)) {
                return child;
            }
        }
    }

    function addGroup(title) {
        var loader = root.findEmptyLoader();

        if (loader) {
            loader.source = Qt.resolvedUrl("PluginSettingsGroupLabel.qml");
            loader.item.text = title;
        }
    }

    function addSelectionItem(key, defaultValue, title, list) {
        var loader = root.findEmptyLoader();

        if (loader) {
            loader.source = Qt.resolvedUrl("PluginSettingsValueSelector.qml");
            loader.item.title = title;
            loader.item.setList(key, defaultValue, list);
        }
    }

    function addSwitch(key, defaultValue, title) {
        var loader = root.findEmptyLoader();

        if (loader) {
            loader.source = Qt.resolvedUrl("PluginSettingsSwitch.qml");
            loader.item.title = title;
            loader.item.setKey(key, defaultValue);
        }
    }

    function addSlider(key, defaultValue, title, min, max, step) {
        var loader = root.findEmptyLoader();

        if (loader) {
            loader.source = Qt.resolvedUrl("PluginSettingsSlider.qml");
            loader.item.title = title;
            loader.item.minimumValue = min;
            loader.item.maximumValue = max;
            loader.item.stepSize = step;
            loader.item.setKey(key, defaultValue);
        }
    }

    function addTextField(key, defaultValue, title) {
        var loader = root.findEmptyLoader();

        if (loader) {
            loader.source = Qt.resolvedUrl("PluginSettingsTextField.qml");
            loader.item.title = title;
            loader.item.setKey(key, defaultValue);
        }
    }

    orientationLock: Settings.screenOrientation
    tools: ToolBarLayout {

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }

    Flickable {
        id: flicker

        anchors.fill: parent
        contentHeight: column.height + UI.PADDING_DOUBLE

        Column {
            id: column

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            TitleHeader {
                id: header
            }

            Repeater {
                id: repeater

                model: 20

                Loader {
                    property string key: ""

                    objectName: "loader" + (modelData + 1)
                    width: column.width
                    height: source == "" ? 0 : 80
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }
}
