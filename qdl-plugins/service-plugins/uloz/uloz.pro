QT += core network
QT -= gui
TARGET = uloz
TEMPLATE = lib

HEADERS += \
    uloz.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    uloz.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
