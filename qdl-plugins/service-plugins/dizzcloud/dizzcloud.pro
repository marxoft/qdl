QT += core network
QT -= gui
TARGET = dizzcloud
TEMPLATE = lib

HEADERS += \
    dizzcloud.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    dizzcloud.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
