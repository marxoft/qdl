QT += core network
QT -= gui
TARGET = fileboom
TEMPLATE = lib

HEADERS += \
    fileboom.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    fileboom.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
