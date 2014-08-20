QT += core network
QT -= gui
TARGET = lafiles
TEMPLATE = lib

HEADERS += \
    lafiles.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    lafiles.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
