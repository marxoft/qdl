QT += core network
QT -= gui
TARGET = pinktube
TEMPLATE = lib

HEADERS += \
    pinktube.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    pinktube.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
