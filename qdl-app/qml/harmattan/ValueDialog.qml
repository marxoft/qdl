import QtQuick 1.1
import com.nokia.meego 1.0
import com.marxoft.models 1.0
import com.marxoft.enums 1.0

SelectionDialog {
    id: root

    property string name
    property variant value

    onAccepted: {
        name = model.data(selectedIndex, "name");
        value = model.data(selectedIndex, "value");
    }

    onStatusChanged:  {
        if (status === DialogStatus.Opening) {
            for (var i = 0; i < model.count; i++) {
                if (model.data(i, "value") === value) {
                    name = model.data(i, "name");
                    selectedIndex = i;

                    return;
                }
            }
        }
    }
}
