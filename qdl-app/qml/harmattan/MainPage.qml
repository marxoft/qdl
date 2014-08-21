import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import com.marxoft.enums 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Page {
    id: root

    orientationLock: Settings.screenOrientation
    tools: ToolBarLayout {

        Label {
            anchors {
                left: parent.left
                leftMargin: UI.PADDING_DOUBLE
            }
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: UI.FONT_SMALL
            font.family: UI.FONT_FAMILY_LIGHT
            text: TransferModel.totalDownloadSpeed + " kB/s"
        }

        ToolIcon {
            platformIconId: "toolbar-mediacontrol-play"
            enabled: (view.visible) && (TransferModel.count > 0)
            opacity: enabled ? UI.OPACITY_ENABLED : UI.OPACITY_DISABLED
            onClicked: TransferModel.start()
        }

        ToolIcon {
            platformIconId: "toolbar-mediacontrol-pause"
            enabled: (view.visible) && (TransferModel.count > 0)
            opacity: enabled ? UI.OPACITY_ENABLED : UI.OPACITY_DISABLED
            onClicked: TransferModel.pause()
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            enabled: view.visible
            opacity: enabled ? UI.OPACITY_ENABLED : UI.OPACITY_DISABLED
            onClicked: actionMenu.open()
        }
    }

    Menu {
        id: actionMenu

        MenuLayout {

            MenuItem {
                text: qsTr("Add URLs")
                onClicked: {
                    dialogLoader.sourceComponent = addUrlsDialog;
                    dialogLoader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Import URLs")
                onClicked: {
                    dialogLoader.sourceComponent = fileBrowserDialog;
                    dialogLoader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Retrieve URLs")
                onClicked: {
                    dialogLoader.sourceComponent = retrieveUrlsDialog;
                    dialogLoader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: appWindow.pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                text: qsTr("About")
                onClicked: {
                    dialogLoader.sourceComponent = aboutDialog;
                    dialogLoader.item.open();
                }
            }
        }
    }

    Menu {
        id: filterMenu

        MenuLayout {

            ValueMenuItem {
                id: statusFilterMenuItem

                title: qsTr("Show")
                subTitle: qsTr("All")
                onClicked: {
                    dialogLoader.sourceComponent = statusFilterDialog;
                    dialogLoader.item.open();
                }
            }

            ValueMenuItem {
                id: nextActionMenuItem

                title: qsTr("After current download(s)")
                subTitle: qsTr("Continue")
                onClicked: {
                    dialogLoader.sourceComponent = nextActionDialog;
                    dialogLoader.item.open();
                }
            }

            ValueMenuItem {
                id: concurrentTransfersMenuItem

                title: qsTr("Concurrent downloads")
                subTitle: Settings.maximumConcurrentTransfers
                onClicked: {
                    dialogLoader.sourceComponent = concurrentTransfersDialog;
                    dialogLoader.item.open();
                }
            }

            ValueMenuItem {
                id: connectionsMenuItem

                title: qsTr("Connections per download")
                subTitle: Settings.maximumConnectionsPerTransfer
                onClicked: {
                    dialogLoader.sourceComponent = connectionsDialog;
                    dialogLoader.item.open();
                }
            }
        }
    }

    SearchBox {
        id: searchBox

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        z: 1000
        placeholderText: qsTr("Search")
        enabled: (view.visible) && (TransferModel.count > 0)
        onSearchTextChanged: TransferModel.searchQuery = searchText
        onMenuTriggered: filterMenu.open()
    }

    ListView {
        id: view

        property int selectedIndex: -1
        property int selectedParentIndex: -1

        anchors {
            top: searchBox.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        model: TransferModel
        delegate: TransferPackageDelegate {}
        visible: (!urlCheckInfo.visible) && (!progressInfo.visible)
    }

    ContextMenu {
        id: contextMenu

        MenuLayout {

            MenuItem {
                text: qsTr("Download properties")
                onClicked: {
                    dialogLoader.sourceComponent = transferPropertiesDialog;
                    dialogLoader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Package properties")
                visible: view.selectedParentIndex === -1
                onClicked: {
                    dialogLoader.sourceComponent = packagePropertiesDialog;
                    dialogLoader.item.open();
                }
            }

            MenuItem {
                text: qsTr("Start")
                onClicked: TransferModel.setData(view.selectedIndex, view.selectedParentIndex, Transfers.Queued, "status")
            }

            MenuItem {
                text: qsTr("Pause")
                onClicked: TransferModel.setData(view.selectedIndex, view.selectedParentIndex, Transfers.Paused, "status")
            }

            MenuItem {
                text: qsTr("Remove")
                onClicked: TransferModel.setData(view.selectedIndex, view.selectedParentIndex, Transfers.Cancelled, "status")
            }
        }
    }

    ScrollDecorator {
        flickableItem: view
    }

    Label {
        anchors.centerIn: view
        font {
            bold: true
            pixelSize: 40
        }
        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
        text: qsTr("No downloads")
        visible: (TransferModel.count == 0) && (view.visible)
    }

    UrlCheckInfo {
        id: urlCheckInfo

        anchors {
            top: searchBox.bottom
            topMargin: UI.PADDING_DOUBLE
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: UI.PADDING_DOUBLE
        }
        visible: false
    }

    ProgressInfo {
        id: progressInfo

        anchors {
            left: parent.left
            leftMargin: 50
            right: parent.right
            rightMargin: 50
            verticalCenter: view.verticalCenter
        }
        visible: false
    }

    Loader {
        id: dialogLoader
    }

    Component {
        id: nextActionDialog

        ValueDialog {
            titleText: qsTr("After current download(s)")
            model: TransferActionModel {}
	    value: TransferModel.nextAction
            onNameChanged: nextActionMenuItem.subTitle = name
            onValueChanged: TransferModel.nextAction = value
        }
    }

    Component {
        id: statusFilterDialog

        ValueDialog {
            titleText: qsTr("Show")
            model: StatusFilterModel {}
            onNameChanged: statusFilterMenuItem.subTitle = name
            onValueChanged: TransferModel.statusFilter = value
            Component.onCompleted: value = TransferModel.statusFilter
        }
    }

    Component {
        id: concurrentTransfersDialog

        ValueDialog {
            titleText: qsTr("Concurrent downloads")
            model: ConcurrentTransfersModel {}
            value: Settings.maximumConcurrentTransfers
            onValueChanged: Settings.maximumConcurrentTransfers = value
        }
    }

    Component {
        id: connectionsDialog

        ValueDialog {
            titleText: qsTr("Connections per download")
            model: ConnectionsModel {}
            value: Settings.maximumConnectionsPerTransfer
            onValueChanged: Settings.maximumConnectionsPerTransfer = value
        }
    }

    Component {
        id: transferPropertiesDialog

        TransferPropertiesDialog {
            transfer: status === DialogStatus.Closed ? null : TransferModel.get(view.selectedIndex, view.selectedParentIndex)
        }
    }

    Component {
        id: packagePropertiesDialog

        PackagePropertiesDialog {
            transfer: status === DialogStatus.Closed ? null : TransferModel.get(view.selectedIndex, -1)
        }
    }

    Component {
        id: addUrlsDialog

        AddUrlsDialog {
            onUrlsAvailable: {
                urlCheckInfo.visible = true;
                UrlChecker.parseUrlsFromText(urls, service);
            }
        }
    }

    Component {
        id: fileBrowserDialog

        FileBrowserDialog {
            startFolder: "/home/user/MyDocs/"
            showFiles: true
            onFileChosen: UrlChecker.importUrlsFromTextFile(filePath)
        }
    }

    Component {
        id: retrieveUrlsDialog

        RetrieveUrlsDialog {
            onUrlsAvailable: {
                urlRetrieverConnections.target = UrlRetriever;
                UrlRetriever.parseUrlsFromText(urls);
            }
        }
    }

    Component {
        id: aboutDialog

        AboutDialog {}
    }

    Connections {
        id: urlRetrieverConnections

        target: null
        onBusy: progressInfo.open(message, numberOfOperations)
        onProgressChanged: progressInfo.updateProgress(progress)
        onFinished: {
            target = null;
            progressInfo.close();
            var results = UrlRetriever.resultsString();

            if (results) {
                dialogLoader.sourceComponent = addUrlsDialog;
                dialogLoader.item.text = results;
                dialogLoader.item.open();
                UrlRetriever.clearResults();
            }
            else {
                banner.displayMessage(qsTr("No supported URLs found"));
            }
        }
    }

    Connections {
        target: PluginManager
        onBusy: progressInfo.open(message, numberOfOperations)
        onProgressChanged: progressInfo.updateProgress(progress)
        onPluginsReady: {
            progressInfo.close();

            if (!TransferModel.count) {
                TransferModel.restoreStoredTransfers();
            }
        }
    }

    Component.onCompleted: PluginManager.loadPlugins()
}
