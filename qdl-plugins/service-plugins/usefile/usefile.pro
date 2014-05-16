QT += core network
QT -= gui
TARGET = usefile
TEMPLATE = lib

HEADERS += \
    usefile.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    usefile.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
