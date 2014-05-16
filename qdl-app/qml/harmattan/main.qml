import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.0

PageStackWindow {
    id: appWindow

    showStatusBar: true
    showToolBar: true
    initialPage: MainPage {
        id: mainPage
    }

    Component.onCompleted: theme.inverted = true

    InfoBanner {
        id: banner

        function displayMessage(message) {
            text = message;
            show();
        }

        topMargin: 40
    }

    Connections {
        target: platformWindow
        onActiveChanged: {
            if (!platformWindow.active) {
                Settings.saveSettings();
                TransferModel.storeTransfers();
            }
        }
    }
}
