QT += core network
QT -= gui
TARGET = filemates
TEMPLATE = lib

HEADERS += \
    filemates.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filemates.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
