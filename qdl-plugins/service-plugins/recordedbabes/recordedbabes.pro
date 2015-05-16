QT += core network
QT -= gui
TARGET = recordedbabes
TEMPLATE = lib

HEADERS += \
    recordedbabes.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    recordedbabes.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
