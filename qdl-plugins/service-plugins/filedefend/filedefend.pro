QT += core network
QT -= gui
TARGET = filedefend
TEMPLATE = lib

HEADERS += \
    filedefend.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filedefend.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
