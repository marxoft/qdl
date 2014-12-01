TEMPLATE = app
TARGET = qdl
QT += network sql xml
INSTALLS += target

HEADERS += $$files(src/shared/*.h) \
    $$files(src/interfaces/*.h)

SOURCES += $$files(src/shared/*.cpp)

#DEFINES += MAEMO4_OS
DEFINES += TABLE_TRANSFER_VIEW

maemo5 {
    DEFINES += WEB_INTERFACE
    QT += maemo5 dbus

    HEADERS += $$files(src/maemo5/*.h) \
        $$files(src/dbus/*.h) \
        $$files(src/captchadialog/*.h)

    SOURCES += $$files(src/maemo5/*.cpp) \
        $$files(src/dbus/*.cpp) \
        $$files(src/captchadialog/*.cpp)

    desktopfile.path = /usr/share/applications/hildon
    desktopfile.files = desktop/maemo5/qdl.desktop

    icon.path = /usr/share/icons/hicolor/64x64/apps
    icon.files = desktop/maemo5/64x64/qdl.png

    backupicon.path = /opt/qdl/icons
    backupicon.files = desktop/maemo5/64x64/qdl.png

    dbusservice.path = /usr/share/dbus-1/services
    dbusservice.files = dbus/com.marxoft.QDL.service

    dbusinterface.path = /usr/share/dbus-1/interfaces
    dbusinterface.files = dbus/com.marxoft.QDL.xml

    INSTALLS += desktopfile icon backupicon dbusservice dbusinterface

    target.path = /opt/qdl/bin

} else:contains(DEFINES,MAEMO4_OS) {
    DEFINES += WEB_INTERFACE
    QT += dbus

    HEADERS += $$files(src/maemo4/*.h) \
        $$files(src/dbus/*.h) \
        $$files(src/captchadialog/*.h)

    SOURCES += $$files(src/maemo4/*.cpp) \
        $$files(src/dbus/*.cpp) \
        $$files(src/captchadialog/*.cpp)

    desktopfile.path = /usr/share/applications/hildon
    desktopfile.files = desktop/maemo4/qdl.desktop

    icon64.path = /usr/share/icons/hicolor/64x64/apps
    icon64.files = desktop/maemo4/64x64/qdl.png

    icon48.path = /usr/share/icons/hicolor/48x48/apps
    icon48.files = desktop/maemo4/48x48/qdl.png

    icon40.path = /usr/share/icons/hicolor/40x40/apps
    icon40.files = desktop/maemo4/40x40/qdl.png

    icon32.path = /usr/share/icons/hicolor/32x32/apps
    icon32.files = desktop/maemo4/32x32/qdl.png

    icon22.path = /usr/share/icons/hicolor/22x22/apps
    icon22.files = desktop/maemo4/22x22/qdl.png

    icon16.path = /usr/share/icons/hicolor/16x16/apps
    icon16.files = desktop/maemo4/16x16/qdl.png

    backupicon.path = /opt/qdl/icons
    backupicon.files = desktop/maemo4/64x64/qdl.png

    dbusservice.path = /usr/share/dbus-1/services
    dbusservice.files = dbus/com.marxoft.QDL.service

    dbusinterface.path = /usr/share/dbus-1/interfaces
    dbusinterface.files = dbus/com.marxoft.QDL.xml

    INSTALLS += desktopfile icon64 icon48 icon40 icon32 icon22 icon16 backupicon dbusservice dbusinterface

    target.path = /opt/qdl/bin

} else:contains(MEEGO_EDITION,harmattan) {
    DEFINES += QML_USER_INTERFACE WEB_INTERFACE
    QT+= opengl declarative dbus
    CONFIG += qdeclarative-boostable

    HEADERS += $$files(src/harmattan/*.h) \
        $$files(src/dbus/*.h)

    SOURCES += $$files(src/harmattan/*.cpp) \
        $$files(src/dbus/*.cpp)

    RESOURCES += qml/harmattan/resources.qrc

    OTHER_FILES += $$files(qml/harmattan/*.qml)

    desktopfile.files = desktop/harmattan/qdl.desktop
    desktopfile.path = /usr/share/applications

    icon.files = desktop/harmattan/80x80/qdl.png
    icon.path = /usr/share/icons/hicolor/80x80/apps

    backupicon.path = /opt/qdl/icons
    backupicon.files = desktop/harmattan/80x80/qdl.png

    splash.files += $$files(splash/harmattan/*.png)
    splash.path = /opt/qdl/splash

    dbusservice.path = /usr/share/dbus-1/services
    dbusservice.files = dbus/harmattan/com.marxoft.QDL.service

    dbusinterface.path = /usr/share/dbus-1/interfaces
    dbusinterface.files = dbus/com.marxoft.QDL.xml

    INSTALLS += desktopfile icon backupicon splash dbusservice dbusinterface

    target.path = /opt/qdl/bin

} else:unix {
    DEFINES += WEB_INTERFACE
    QT += dbus

    HEADERS += $$files(src/desktop/*.h) \
        $$files(src/dbus/*.h) \
        $$files(src/captchadialog/*.h)

    SOURCES += $$files(src/desktop/*.cpp) \
        $$files(src/dbus/*.cpp) \
        $$files(src/captchadialog/*.cpp)

    desktopfile.path = /usr/share/applications
    desktopfile.files = desktop/desktop/qdl.desktop

    icon64.path = /usr/share/icons/hicolor/64x64/apps
    icon64.files = desktop/desktop/64x64/qdl.png

    icon48.path = /usr/share/icons/hicolor/48x48/apps
    icon48.files = desktop/desktop/48x48/qdl.png

    icon22.path = /usr/share/icons/hicolor/22x22/apps
    icon22.files = desktop/desktop/22x22/qdl.png

    icon16.path = /usr/share/icons/hicolor/16x16/apps
    icon16.files = desktop/desktop/16x16/qdl.png

    backupicon.path = /opt/qdl/icons
    backupicon.files = desktop/desktop/64x64/qdl.png

    dbusservice.path = /usr/share/dbus-1/services
    dbusservice.files = dbus/com.marxoft.QDL.service

    dbusinterface.path = /usr/share/dbus-1/interfaces
    dbusinterface.files = dbus/com.marxoft.QDL.xml

    INSTALLS += desktopfile icon64 icon48 icon22 icon16 backupicon dbusservice dbusinterface

    target.path = /opt/qdl/bin
}

contains(DEFINES,WEB_INTERFACE) {
    HEADERS += $$files(src/webif/*.h) \
        $$files(src/qhttpserver/*.h) \
        $$files(src/json/*.h)
    SOURCES += $$files(src/webif/*.cpp) \
        $$files(src/qhttpserver/*.cpp) \
        $$files(src/qhttpserver/*.c) \
        $$files(src/json/*.cpp)

    webif.files += webif
    webif.path = /opt/qdl

    INSTALLS += webif
}
