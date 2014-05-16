QT += core network
QT -= gui
TARGET = netload
TEMPLATE = lib

HEADERS += \
    netload.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    netload.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
