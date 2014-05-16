QT += core network
QT -= gui
TARGET = filestay
TEMPLATE = lib

HEADERS += \
    filestay.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filestay.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
