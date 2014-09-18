QT += core network
QT -= gui
TARGET = blst
TEMPLATE = lib

HEADERS += \
    blst.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    blst.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
