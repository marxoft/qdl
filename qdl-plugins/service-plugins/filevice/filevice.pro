QT += core network
QT -= gui
TARGET = filevice
TEMPLATE = lib

HEADERS += \
    filevice.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filevice.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
