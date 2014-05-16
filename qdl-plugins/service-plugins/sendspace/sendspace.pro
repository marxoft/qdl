QT += core network
QT -= gui
TARGET = sendspace
TEMPLATE = lib

HEADERS += \
    sendspace.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    sendspace.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
