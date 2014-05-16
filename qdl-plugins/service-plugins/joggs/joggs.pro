QT += core network
QT -= gui
TARGET = joggs
TEMPLATE = lib

HEADERS += \
    joggs.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    joggs.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
