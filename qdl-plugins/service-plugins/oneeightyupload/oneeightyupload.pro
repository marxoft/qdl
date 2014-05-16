QT += core network
QT -= gui
TARGET = oneeightyupload
TEMPLATE = lib

HEADERS += \
    oneeightyupload.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    oneeightyupload.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
